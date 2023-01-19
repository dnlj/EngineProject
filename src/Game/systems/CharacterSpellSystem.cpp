// STD
#include <algorithm>

// Engine
#include <Engine/Glue/Box2D.hpp>

// Game
#include <Game/World.hpp>
#include <Game/systems/CharacterSpellSystem.hpp>
#include <Game/systems/PhysicsSystem.hpp>
#include <Game/systems/NetworkingSystem.hpp>
#include <Game/comps/SpriteComponent.hpp>
#include <Game/comps/PhysicsBodyComponent.hpp>
#include <Game/comps/PhysicsInterpComponent.hpp>
#include <Game/comps/ActionComponent.hpp>


namespace {
	using namespace Game;
	using Engine::ECS::Entity;
	using Engine::Net::MessageHeader;
	using Engine::Net::BufferReader;

	// TODO: Should probably be derived from actionsystem inputs/binds/w.e. not its own message
	void recv_SPELL(EngineInstance& engine, ConnectionInfo& from, const MessageHeader head, BufferReader& msg) {
		b2Vec2 pos;
		b2Vec2 dir;
		if (!msg.read(&pos) || !msg.read(&dir)) { return; }
		auto& world = engine.getWorld();
		auto& spellSys = world.getSystem<CharacterSpellSystem>();
		spellSys.queueMissile(pos, dir);
	}
}


namespace Game {
	CharacterSpellSystem::CharacterSpellSystem(SystemArg arg)
		: System{arg} {
		static_assert(World::orderAfter<CharacterSpellSystem, CharacterMovementSystem>());
		static_assert(World::orderAfter<CharacterSpellSystem, PhysicsSystem>());
	}

	void CharacterSpellSystem::setup() {
		auto& netSys = world.getSystem<NetworkingSystem>();
		netSys.setMessageHandler(MessageType::SPELL, recv_SPELL);

		auto& physSys = world.getSystem<Game::PhysicsSystem>();
		physSys.addListener(this);

		constexpr std::size_t count = 10;
		missiles.reserve(count);
		
		for (int i = 0; i < count; ++i) {
			auto ent = missiles.emplace_back(world.createEntity(true));
			const auto& [spriteComp, physBodyComp, physInterpComp] = world.addComponents<
				Game::SpriteComponent,
				Game::PhysicsBodyComponent,
				Game::PhysicsInterpComponent
			>(ent);

			{
				b2BodyDef bodyDef;
				bodyDef.type = b2_dynamicBody;
				bodyDef.position = b2Vec2_zero;

				physBodyComp.setBody(physSys.createBody(ent, bodyDef));

				b2CircleShape shape;
				shape.m_radius = 0.5f;

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &shape;
				fixtureDef.density = 0.0f;
				fixtureDef.isSensor = true;

				physBodyComp.getBody().CreateFixture(&fixtureDef);
				physBodyComp.getBody().SetLinearDamping(0.0f);
				physBodyComp.getBody().SetFixedRotation(true);
				physBodyComp.getBody().SetActive(false);
			}

			spriteComp.path = "assets/fire.png";
			spriteComp.texture = engine.getTextureLoader().get2D(spriteComp.path);
			world.setEnabled(ent, false);
		}
	}
	
	void CharacterSpellSystem::queueMissile(const b2Vec2& pos, const b2Vec2& dir) {
		events.push_back(FireEvent{
			.pos = pos,
			.dir = dir,
		});
	}

	void CharacterSpellSystem::fireMissile(const b2Vec2& pos, const b2Vec2& dir) {
		auto missile = missiles[currentMissile];
		world.setEnabled(missile, true);

		auto& physComp = world.getComponent<PhysicsBodyComponent>(missile);
		physComp.snap = true;
		physComp.setTransform(pos, 0);

		auto& body = physComp.getBody();
		body.SetActive(true); // TODO: use physComp instead
		physComp.setVelocity(4.0f * dir);

		currentMissile = (currentMissile + 1) % missiles.size();
	}

	void CharacterSpellSystem::detonateMissile(Engine::ECS::Entity ent) {
		std::cout << "Boom: " << ent << "\n";
		world.setEnabled(ent, false);
		world.getComponent<PhysicsBodyComponent>(ent).getBody().SetActive(false);
	}

	void CharacterSpellSystem::tick() {
		for (const auto ent : world.getFilter<PhysicsBodyComponent, ActionComponent>()) {
			auto& actComp = world.getComponent<ActionComponent>(ent);

			if (actComp.getAction(Action::Attack1).pressCount) {
				const auto& physComp = world.getComponent<PhysicsBodyComponent>(ent);
				const auto pos = physComp.getPosition();
				auto dir = Engine::Glue::as<b2Vec2>(actComp.getTarget());
				dir.Normalize();

				queueMissile(pos + 1.3f * dir, 4.0f * dir);
				events.back().ent = ent;
			}
		}

		if (!toDestroy.empty()) {
			std::sort(toDestroy.begin(), toDestroy.end());

			while (!toDestroy.empty()) {
				auto ent = toDestroy.back();
				toDestroy.pop_back();
				detonateMissile(ent);

				while(!toDestroy.empty() && toDestroy.back() == ent) {
					toDestroy.pop_back();
				}
			}
		}

		for (const auto& event : events) {
			fireMissile(event.pos, event.dir);

			if constexpr (ENGINE_SERVER) {
				auto& netSys = world.getSystem<NetworkingSystem>();

				for (const auto ply : world.getFilter<PlayerFlag, ConnectedFlag>()) {
					if (ply == event.ent) { continue; }
					auto* conn = netSys.getConnection(ply);

					if (!conn) {
						ENGINE_WARN("Unable to get network connection. This is a bug.");
						continue;
					}

					if (auto msg = conn->beginMessage<MessageType::SPELL>()) {
						msg.write(event.pos);
						msg.write(event.dir);
					}
				}
			}
		}
		events.clear();
	}

	void CharacterSpellSystem::beginContact(const Engine::ECS::Entity& entA, const Engine::ECS::Entity& entB) {
		const auto minEnt = missiles.front();
		const auto maxEnt = missiles.back();

		if (entA <= maxEnt && entA >= minEnt) {
			toDestroy.push_back(entA);
		}

		if (entB <= maxEnt && entB >= minEnt) {
			toDestroy.push_back(entB);
		}
	}
}
