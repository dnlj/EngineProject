// GLM
#include <glm/gtc/matrix_transform.hpp>

// Game
#include <Game/CameraTrackingSystem.hpp>

namespace Game {
	CameraTrackingSystem::CameraTrackingSystem(World& world) : SystemBase{world} {
		priorityAfter = world.getBitsetForSystems<Game::InputSystem>();
	}

	void CameraTrackingSystem::setup(Engine::Camera& camera) {
		this->camera = &camera;
	}

	void CameraTrackingSystem::run(float dt) {
		const auto focusPos = world.getComponent<PhysicsComponent>(focus).body->GetPosition();
		camera->view = glm::translate(glm::mat4{1.0f}, glm::vec3{-focusPos.x, -focusPos.y, 0.0f});
	}
}
