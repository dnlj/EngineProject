// STD
#include <algorithm>

// Game
#include <Game/CharacterSpellSystem.hpp>
#include <Game/PhysicsSystem.hpp>
#include <Game/PhysicsComponent.hpp>
#include <Game/SpriteComponent.hpp>
#include <Game/InputComponent.hpp>
#include <Game/World.hpp>


namespace Game {
	CharacterSpellSystem::CharacterSpellSystem(SystemArg arg)
		: SystemBase{arg} {

		// TODO: Add static check for: priorityAfter = world.getBitsetForSystems<Game::CharacterMovementSystem>();
		// TODO: Add static check for: priorityBefore = world.getBitsetForSystems<Game::PhysicsSystem>();
	}

	void CharacterSpellSystem::setup(Engine::EngineInstance& engine) {
		auto& physSys = world.getSystem<Game::PhysicsSystem>();
		physSys.addListener(this);

		constexpr std::size_t count = 10;
		missles.reserve(count);
		
		for (int i = 0; i < count; ++i) {
			auto ent = missles.emplace_back(world.createEntity(true));
			auto& [spriteComp, physComp] = world.addComponents<
				Game::SpriteComponent,
				Game::PhysicsComponent
			>(ent);

			{
				b2BodyDef bodyDef;
				bodyDef.type = b2_dynamicBody;
				bodyDef.position = b2Vec2_zero;

				physComp.physSys = &world.getSystem<Game::PhysicsSystem>();
				physComp.body = physComp.physSys->createBody(ent, bodyDef);

				b2CircleShape shape;
				shape.m_radius = 1.0f/8;

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &shape;
				fixtureDef.density = 0.0f;
				fixtureDef.isSensor = true;

				physComp.body->CreateFixture(&fixtureDef);
				physComp.body->SetLinearDamping(0.0f);
				physComp.body->SetFixedRotation(true);
				physComp.body->SetActive(false);
			}

			spriteComp.texture = engine.textureManager.get("../assets/fire.png");
			world.setEnabled(ent, false);
		}
	}

	void CharacterSpellSystem::fireMissile(const b2Vec2& pos, const b2Vec2& dir) {
		auto missle = missles[currentMissle];

		world.setEnabled(missle, true);
		auto* body = world.getComponent<Game::PhysicsComponent>(missle).body;
		body->SetActive(true);
		body->SetTransform(pos, 0);
		body->SetLinearVelocity(2.0f * dir);

		currentMissle = (currentMissle + 1) % missles.size();
	}

	void CharacterSpellSystem::detonateMissle(Engine::ECS::Entity ent) {
		std::cout << "Boom: " << ent << "\n";
		world.setEnabled(ent, false);
		world.getComponent<Game::PhysicsComponent>(ent).body->SetActive(false);
	}

	void CharacterSpellSystem::run(float dt) {
		if (toDestroy.empty()) { return; }

		std::sort(toDestroy.begin(), toDestroy.end());

		while (!toDestroy.empty()) {
			auto ent = toDestroy.back();
			toDestroy.pop_back();
			detonateMissle(ent);

			while(!toDestroy.empty() && toDestroy.back() == ent) {
				toDestroy.pop_back();
			}
		}
	}

	void CharacterSpellSystem::beginContact(const PhysicsUserData& dataA, const PhysicsUserData& dataB) {
		const auto entA = dataA.ent;
		const auto entB = dataB.ent;
		const auto minEnt = missles.front();
		const auto maxEnt = missles.back();

		if (entA <= maxEnt && entA >= minEnt) {
			toDestroy.push_back(entA);
		}

		if (entB <= maxEnt && entB >= minEnt) {
			toDestroy.push_back(entB);
		}
	}
}
