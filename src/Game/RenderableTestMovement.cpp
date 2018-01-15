// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// GLFW
#include <GLFW/glfw3.h>

// Game
#include <Game/RenderableTestMovement.hpp>
#include <Game/RenderableTestSystem.hpp>
#include <Game/PhysicsComponent.hpp>

extern GLFWwindow* window; // TODO: Add a way to pass data to systems

namespace Game {
	RenderableTestMovement::RenderableTestMovement() {
		cbits = Engine::ECS::getBitsetForComponents<Game::PhysicsComponent>();
		priorityBefore = Engine::ECS::getBitsetForSystems<Game::RenderableTestSystem>();
	}

	void RenderableTestMovement::run(float dt) {
		constexpr float speed = 1.0f;
		for (auto& ent : entities) {
			auto& physComp = ent.getComponent<Game::PhysicsComponent>();

			// TODO: this should work the other way. Apply force to the body then update the draw position.
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

	ENGINE_REGISTER_SYSTEM(RenderableTestMovement);
}
