// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// GLFW
#include <GLFW/glfw3.h>

// Game
#include <Game/CharacterMovementSystem.hpp>
#include <Game/RenderSystem.hpp>
#include <Game/PhysicsComponent.hpp>
#include <Game/CharacterMovementComponent.hpp>


namespace Game {
	CharacterMovementSystem::CharacterMovementSystem(World& world) : SystemBase{world} {
		cbits = world.getBitsetForComponents<
			Game::PhysicsComponent,
			Game::CharacterMovementComponent
		>();

		priorityAfter = world.getBitsetForSystems<Game::InputSystem>();
		priorityBefore = world.getBitsetForSystems<Game::RenderSystem>();
	}

	void CharacterMovementSystem::run(float dt) {
		constexpr float speed = 2.0f;
		for (auto eid : entities) {
			auto& physComp = world.getComponent<Game::PhysicsComponent>(eid);
			auto& inputComp = world.getComponent<Game::InputComponent>(eid);
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
