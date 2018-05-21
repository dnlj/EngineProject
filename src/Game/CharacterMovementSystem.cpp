// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// GLFW
#include <GLFW/glfw3.h>

// Game
#include <Game/CharacterMovementSystem.hpp>
#include <Game/RenderSystem.hpp>
#include <Game/PhysicsComponent.hpp>

extern GLFWwindow* window; // TODO: Add a way to pass data to systems

namespace Game {
	CharacterMovementSystem::CharacterMovementSystem(World& world) : SystemBase{world} {
		cbits = world.getBitsetForComponents<Game::PhysicsComponent>();
		priorityBefore = world.getBitsetForSystems<Game::RenderSystem>();
	}

	void CharacterMovementSystem::run(float dt) {
		constexpr float speed = 1.0f;
		for (auto eid : entities) {
			auto& physComp = world.getComponent<Game::PhysicsComponent>(eid);
		
			if (glfwGetKey(window, GLFW_KEY_W)) {
				physComp.body->ApplyLinearImpulseToCenter(b2Vec2{0.0f, speed * dt}, true);
			}
		
			if (glfwGetKey(window, GLFW_KEY_S)) {
				physComp.body->ApplyLinearImpulseToCenter(b2Vec2{0.0f, -speed * dt}, true);
			}
		
			if (glfwGetKey(window, GLFW_KEY_A)) {
				physComp.body->ApplyLinearImpulseToCenter(b2Vec2{-speed * dt, 0.0f}, true);
			}
		
			if (glfwGetKey(window, GLFW_KEY_D)) {
				physComp.body->ApplyLinearImpulseToCenter(b2Vec2{speed * dt, 0.0f}, true);
			}
		}
	}
}
