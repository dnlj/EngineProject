// GLM
#include <glm/gtc/matrix_transform.hpp>

// Game
#include <Game/CameraTrackingSystem.hpp>

namespace Game {
	CameraTrackingSystem::CameraTrackingSystem(World& world) : SystemBase{world} {
	}

	void CameraTrackingSystem::setup(Engine::Camera& camera) {
		this->camera = &camera;
	}

	void CameraTrackingSystem::run(float dt) {
		const auto focusPos = world.getComponent<PhysicsComponent>(focus).body->GetPosition();
		camera->setPosition(glm::vec2{focusPos.x, focusPos.y});
	}
}
