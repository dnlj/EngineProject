// STD
#include <algorithm>

// Game
#include <Game/CharacterSpellSystem.hpp>
#include <Game/PhysicsSystem.hpp>
#include <Game/PhysicsComponent.hpp>
#include <Game/SpriteComponent.hpp>
#include <Game/World.hpp>


namespace Game {
	CharacterSpellSystem::CharacterSpellSystem(SystemArg arg)
		: System{arg} {
		static_assert(World::orderAfter<CharacterSpellSystem, CharacterMovementSystem>());
		static_assert(World::orderAfter<CharacterSpellSystem, PhysicsSystem>());
	}

	void CharacterSpellSystem::setup() {
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

				physComp.setBody(physSys.createBody(ent, bodyDef));

				b2CircleShape shape;
				shape.m_radius = 1.0f/8;

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &shape;
				fixtureDef.density = 0.0f;
				fixtureDef.isSensor = true;

				physComp.getBody().CreateFixture(&fixtureDef);
				physComp.getBody().SetLinearDamping(0.0f);
				physComp.getBody().SetFixedRotation(true);
				physComp.getBody().SetActive(false);
			}

			spriteComp.texture = engine.textureManager.get("../assets/fire.png");
			world.setEnabled(ent, false);
		}
	}

	void CharacterSpellSystem::fireMissile(const b2Vec2& pos, const b2Vec2& dir) {
		auto missle = missles[currentMissle];

		world.setEnabled(missle, true);
		auto& physComp = world.getComponent<Game::PhysicsComponent>(missle);
		physComp.setTransform(pos, 0);

		auto& body = physComp.getBody();
		body.SetActive(true);
		body.SetLinearVelocity(4.0f * dir);

		currentMissle = (currentMissle + 1) % missles.size();
	}

	void CharacterSpellSystem::detonateMissle(Engine::ECS::Entity ent) {
		std::cout << "Boom: " << ent << "\n";
		world.setEnabled(ent, false);
		world.getComponent<Game::PhysicsComponent>(ent).getBody().SetActive(false);
	}

	void CharacterSpellSystem::tick(float dt) {
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
