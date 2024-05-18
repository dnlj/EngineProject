// STD
#include <algorithm>

// Engine
#include <Engine/Glue/Box2D.hpp>

// Game
#include <Game/World.hpp>
#include <Game/systems/CharacterSpellSystem.hpp>
#include <Game/systems/PhysicsSystem.hpp>
#include <Game/systems/NetworkingSystem.hpp>
#include <Game/comps/ActionComponent.hpp>
#include <Game/comps/NetworkComponent.hpp>
#include <Game/comps/PhysicsBodyComponent.hpp>
#include <Game/comps/PhysicsInterpComponent.hpp>
#include <Game/comps/SpriteComponent.hpp>


namespace {
	using namespace Game;
	using Engine::ECS::Entity;
	using Engine::Net::MessageHeader;
	using Engine::Net::BufferReader;

	#if ENGINE_CLIENT
	// TODO: Should probably be derived from actionsystem inputs/binds/w.e. not its own message
	void recv_SPELL(EngineInstance& engine, ConnectionInfo& from, const MessageHeader head, BufferReader& msg) {
		b2Vec2 pos;
		b2Vec2 dir;
		if (!msg.read(&pos) || !msg.read(&dir)) { return; }
		auto& world = engine.getWorld();
		auto& spellSys = world.getSystem<CharacterSpellSystem>();

		// TODO: pull zone id here.
		const auto zoneId = world.getComponent<PhysicsBodyComponent>(from.ent).getZoneId();

		spellSys.queueMissile({
			.ent = {}, // TODO: what to do here? guess we need to send this? not right now, we don't use it.
			.zoneId = zoneId,
			.pos = pos,
			.dir = dir,
		});
	}
	#endif // ENGINE_CLIENT
}


namespace Game {
	CharacterSpellSystem::CharacterSpellSystem(SystemArg arg)
		: System{arg} {
		static_assert(World::orderAfter<CharacterSpellSystem, CharacterMovementSystem>());
		static_assert(World::orderAfter<CharacterSpellSystem, PhysicsSystem>());
	}

	void CharacterSpellSystem::setup() {
		ENGINE_CLIENT_ONLY(
			auto& netSys = world.getSystem<NetworkingSystem>();
			netSys.setMessageHandler(MessageType::SPELL, recv_SPELL)
		);

		auto& physSys = world.getSystem<Game::PhysicsSystem>();
		physSys.addListener(this);

		constexpr std::size_t count = 10;
		missiles.reserve(count);
		
		for (int i = 0; i < count; ++i) {
			auto ent = missiles.emplace_back(world.createEntity(true));
			auto& spriteComp = world.addComponent<SpriteComponent>(ent);
			world.addComponent<PhysicsInterpComponent>(ent);

			{
				b2BodyDef bodyDef;
				bodyDef.type = b2_dynamicBody;
				bodyDef.position = b2Vec2_zero;
				bodyDef.linearDamping = 0.0f;
				bodyDef.fixedRotation = true;
				bodyDef.active = false;
				//bodyDef.bullet = true; // TODO: when should this be used?
				
				auto& physBodyComp = world.addComponent<PhysicsBodyComponent>(ent, physSys.createBody(ent, bodyDef, 0));

				b2CircleShape shape;
				shape.m_radius = 0.5f;

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &shape;
				fixtureDef.density = 0.0f;
				fixtureDef.isSensor = true;

				physBodyComp.createFixture(fixtureDef);
			}

			spriteComp.path = "assets/fire.png";
			spriteComp.texture = engine.getTextureLoader().get2D(spriteComp.path);
			world.setEnabled(ent, false);
		}
	}

	void CharacterSpellSystem::fireMissile(const FireEvent& event) {

		auto missile = missiles[currentMissile];
		world.setEnabled(missile, true);

		auto& physComp = world.getComponent<PhysicsBodyComponent>(missile);

		if (event.zoneId != physComp.getZoneId()) {
			physComp.setZone(event.zoneId);
		}

		physComp.snap = true;
		physComp.setTransform(event.pos, 0);
		physComp.setActive(true);
		physComp.setVelocity(4.0f * event.dir);

		currentMissile = (currentMissile + 1) % missiles.size();
	}

	void CharacterSpellSystem::detonateMissile(Engine::ECS::Entity ent) {
		ENGINE_LOG2("Boom: {}", ent);
		world.setEnabled(ent, false);
		world.getComponent<PhysicsBodyComponent>(ent).setActive(false);
	}

	void CharacterSpellSystem::tick() {
		for (const auto ent : world.getFilter<PhysicsBodyComponent, ActionComponent>()) {
			auto& actComp = world.getComponent<ActionComponent>(ent);

			if (actComp.getAction(Action::Attack1).pressCount) {
				const auto& physComp = world.getComponent<PhysicsBodyComponent>(ent);
				const auto pos = physComp.getPosition();
				auto dir = Engine::Glue::as<b2Vec2>(actComp.getTarget());
				dir.Normalize();

				queueMissile({
					.ent = ent,
					.zoneId = physComp.getZoneId(),
					.pos = pos + 1.3f * dir,
					.dir = 4.0f * dir,
				});
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

		// TODO: Should probably rework this and figure out how to handle events
		//       in more general, how do they interact with rollback, networking, etc.
		for (const auto& event : events) {
			fireMissile(event);

			if constexpr (ENGINE_SERVER) {
				// TODO: Only need to send to players in the same zone. Should probably be batching these all into one message.
				for (const auto ply : world.getFilter<PlayerFlag>()) {
					if (ply == event.ent) { continue; }
					auto& conn = world.getComponent<NetworkComponent>(ply).get();
					ENGINE_DEBUG_ONLY(conn._debug_AllowMessages = true); // Workaround until we rework events, see "events" loop comment.
					if (auto msg = conn.beginMessage<MessageType::SPELL>()) {
						msg.write(event.pos);
						msg.write(event.dir);
					}
					ENGINE_DEBUG_ONLY(conn._debug_AllowMessages = false);
				}
			}
		}
		events.clear();
	}
	
	void CharacterSpellSystem::queueMissile(const FireEvent& event) {
		events.emplace_back(event);
	}

	void CharacterSpellSystem::beginContact(const Engine::ECS::Entity entA, const Engine::ECS::Entity entB) {
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
