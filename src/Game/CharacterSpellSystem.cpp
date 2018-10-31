// STD
#include <algorithm>

// Game
#include <Game/CharacterSpellSystem.hpp>
#include <Game/PhysicsSystem.hpp>
#include <Game/PhysicsComponent.hpp>
#include <Game/SpriteComponent.hpp>
#include <Game/InputComponent.hpp>


namespace Game {
	CharacterSpellSystem::CharacterSpellSystem(World& world)
		: SystemBase{world}
		, filter{world.getFilterFor<
			Game::CharacterSpellComponent,
			Game::InputComponent>()} {

		priorityAfter = world.getBitsetForSystems<Game::CharacterMovementSystem>();
		priorityBefore = world.getBitsetForSystems<Game::PhysicsSystem>();
	}

	void CharacterSpellSystem::setup(Engine::EngineInstance& engine) {
		camera = &engine.camera;
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

	void CharacterSpellSystem::fireMissile(Engine::ECS::Entity ent, Engine::InputManager& inputManager) {
		auto& entBody = *world.getComponent<Game::PhysicsComponent>(ent).body;
		auto missle = missles[currentMissle];

		// Get the mouse position in world space
		auto mousePos = camera->screenToWorld(inputManager.getMousePosition());

		// The direction of the cursor relative to ent
		auto entPos = entBody.GetPosition();
		auto dir = b2Vec2(mousePos.x - entPos.x, mousePos.y - entPos.y);
		dir.Normalize();

		// Fire the missile
		world.setEnabled(missle, true);
		auto* body = world.getComponent<Game::PhysicsComponent>(missle).body;
		body->SetActive(true);
		body->SetTransform(entPos + 0.35f * dir, 0); // TODO: This scalar depends on the size of ent and missle. Handle this better.
		body->SetLinearVelocity(2.0f * dir);

		currentMissle = (currentMissle + 1) % missles.size();
	}

	void CharacterSpellSystem::detonateMissle(Engine::ECS::Entity ent) {
		std::cout << "Boom: " << ent << "\n";
		world.setEnabled(ent, false);
		world.getComponent<Game::PhysicsComponent>(ent).body->SetActive(false);
	}

	void CharacterSpellSystem::run(float dt) {
		for (auto ent : filter) {
			auto& inputManager = *world.getComponent<Game::InputComponent>(ent).inputManager;

			if (inputManager.wasPressed("Spell_1")) {
				fireMissile(ent, inputManager);
			}
		}

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
