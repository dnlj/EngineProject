// STD
#include <algorithm>

// Game
#include <Game/CharacterSpellSystem.hpp>


namespace Game {
	CharacterSpellSystem::CharacterSpellSystem(World& world)
		: SystemBase{world}
		, collisionListener{*this}
		, filter{world.getFilterFor<
			Game::CharacterSpellComponent,
			Game::InputComponent>()} {

		priorityAfter = world.getBitsetForSystems<Game::CharacterMovementSystem>();
		priorityBefore = world.getBitsetForSystems<Game::PhysicsSystem>();
	}

	void CharacterSpellSystem::setup(Engine::EngineInstance& engine) {
		camera = &engine.camera;
		auto& physSys = world.getSystem<Game::PhysicsSystem>();
		physSys.addListener(&collisionListener);

		constexpr std::size_t count = 10;
		missles.reserve(count);
		
		for (int i = 0; i < 10; ++i) {
			auto ent = missles.emplace_back(world.createEntity(true));

			auto& [physComp, spriteComp] = world.addComponents<
				Game::PhysicsComponent,
				Game::SpriteComponent
			>(ent);

			{
				b2BodyDef bodyDef;
				bodyDef.type = b2_kinematicBody;
				bodyDef.position = b2Vec2_zero;

				physComp.body = physSys.createBody(ent, bodyDef);

				b2CircleShape shape;
				shape.m_radius = 1.0f/8;

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &shape;
				fixtureDef.density = 0.0f;
				fixtureDef.isSensor = true;

				physComp.body->CreateFixture(&fixtureDef);
				physComp.body->SetLinearDamping(0.0f);
				physComp.body->SetFixedRotation(true);
			}

			spriteComp.texture = engine.textureManager.getTexture("../assets/fire.png");
		}
	}

	void CharacterSpellSystem::fireMissile(Engine::ECS::Entity ent, Engine::InputManager& inputManager) {
		auto& entBody = *world.getComponent<Game::PhysicsComponent>(ent).body;
		auto missle = missles[currentMissle];

		// Get the mouse position in world space
		auto mousePos = inputManager.getMousePosition(); // Mouse position relative to window
		mousePos -= glm::vec2(camera->getWidth() * 0.5f, camera->getHeight() * 0.5f); // Relative to center of screen
		mousePos.y *= -1.0f; // In screen space up is negative. In world space up is positive
		mousePos = glm::vec2(camera->getPosition()) + mousePos; // Offset by camera position

		// The direction of the cursor relative to ent
		auto entPos = entBody.GetPosition();
		auto dir = b2Vec2(mousePos.x - entPos.x, mousePos.y - entPos.y);
		dir.Normalize();

		// Fire the missile
		auto body = world.getComponent<Game::PhysicsComponent>(missle).body;
		body->SetTransform(entPos + 0.25f * dir, 0); // TODO: This scalar depends on the size of ent and missle. Handle this better.
		body->SetLinearVelocity(2.0f * dir);
		currentMissle = (currentMissle + 1) % missles.size();
	}

	void CharacterSpellSystem::detonateMissle(Engine::ECS::Entity ent) {
		std::cout << "Boom: " << ent << "\n";
		world.removeComponent<Game::PhysicsComponent>(ent);
	}

	void CharacterSpellSystem::run(float dt) {
		for (auto ent : filter) {
			auto& inputManager = *world.getComponent<Game::InputComponent>(ent).inputManager;

			if (inputManager.wasPressed("Spell_1")) {
				fireMissile(ent, inputManager);
			}
		}

		while (!toDestroy.empty()) {
			auto ent = toDestroy.back();
			detonateMissle(ent);
			toDestroy.pop_back();
		}
	}
}

namespace Game {
	CharacterSpellSystem::CollisionListener::CollisionListener(CharacterSpellSystem& spellSys)
		: spellSys{spellSys} {
	}

	void CharacterSpellSystem::CollisionListener::beginContact(const PhysicsUserData& dataA, const PhysicsUserData& dataB) {
		const auto entA = dataA.ent;
		const auto entB = dataB.ent;
		const auto minEnt = spellSys.missles.front();
		const auto maxEnt = spellSys.missles.back();
		auto& con = spellSys.toDestroy;

		if (entA <= maxEnt && entA >= minEnt) {
			if (std::find(con.begin(), con.end(), entA) == con.end()) {
				con.push_back(entA);
			}
		}

		if (entB <= maxEnt && entB >= minEnt) {
			if (std::find(con.begin(), con.end(), entB) == con.end()) {
				con.push_back(entB);
			}
		}
	}
}
