// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// GLFW
#include <GLFW/glfw3.h>

// Game
#include <Game/CharacterMovementSystem.hpp>
#include <Game/PhysicsComponent.hpp>
#include <Game/CharacterMovementComponent.hpp>


namespace Game {
	CharacterMovementSystem::CharacterMovementSystem(World& world) : SystemBase{world} {
		cbits = world.getBitsetForComponents<
			Game::PhysicsComponent,
			Game::CharacterMovementComponent
		>();

		priorityBefore = world.getBitsetForSystems<Game::PhysicsSystem>();
	}

	void CharacterMovementSystem::run(float dt) {
		constexpr float speed = 1.0f * 2;
		for (auto ent : entities) {
			auto& physComp = world.getComponent<Game::PhysicsComponent>(ent);
			auto& inputComp = world.getComponent<Game::InputComponent>(ent);
			auto& inputManager = *inputComp.inputManager;
		
			if (inputManager.isPressed("MoveUp")) {
				physComp.body->ApplyLinearImpulseToCenter(b2Vec2{0.0f, speed * dt}, true);
			}
		
			if (inputManager.isPressed("MoveDown")) {
				physComp.body->ApplyLinearImpulseToCenter(b2Vec2{0.0f, -speed * dt}, true);
			}
		
			if (inputManager.isPressed("MoveLeft")) {
				physComp.body->ApplyLinearImpulseToCenter(b2Vec2{-speed * dt, 0.0f}, true);
			}
		
			if (inputManager.isPressed("MoveRight")) {
				physComp.body->ApplyLinearImpulseToCenter(b2Vec2{speed * dt, 0.0f}, true);
			}
		}
	}
}
