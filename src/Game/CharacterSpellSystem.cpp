// Game
#include <Game/CharacterSpellSystem.hpp>

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
		auto& physWorld = world.getSystem<Game::PhysicsSystem>().getPhysicsWorld();
		
		missles.reserve(10);
		
		for (int i = 0; i < 10; ++i) {
			auto ent = missles.emplace_back(world.createEntity(true));

			auto& [physComp, spriteComp] = world.addComponents<
				Game::PhysicsComponent,
				Game::SpriteComponent
			>(ent);

			{
				b2BodyDef bodyDef;
				bodyDef.type = b2_dynamicBody;
				bodyDef.position = b2Vec2_zero;

				physComp.body = physWorld.CreateBody(&bodyDef);

				b2CircleShape shape;
				shape.m_radius = 1.0f/8;

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &shape;
				fixtureDef.density = 1.0f;

				physComp.body->CreateFixture(&fixtureDef);
				physComp.body->SetLinearDamping(10.0f);
				physComp.body->SetFixedRotation(true);
			}

			spriteComp.texture = engine.textureManager.getTexture("../assets/fire.png");
		}
	}

	void CharacterSpellSystem::run(float dt) {
		for (auto ent : filter) {
			auto& inputManager = *world.getComponent<Game::InputComponent>(ent).inputManager;

			if (inputManager.wasPressed("Spell_1")) {
				auto& entBody = *world.getComponent<Game::PhysicsComponent>(ent).body;
				auto missle = missles[currentMissle];
				auto mousePos = inputManager.getMousePosition();
				auto entPos = entBody.GetPosition();
				auto dir = b2Vec2(mousePos.x - entPos.x, mousePos.y - entPos.y);
				dir.Normalize();

				// TODO: Mouse pos is wrong since its not in world coords

				std::cout << "x: " << mousePos.x;
				std::cout << "\ny: " << mousePos.y << "\n";

				auto body = world.getComponent<Game::PhysicsComponent>(missle).body;
				body->SetTransform(entPos, 0);
				currentMissle = (currentMissle + 1) % missles.size();
			}
		}
	}
}
