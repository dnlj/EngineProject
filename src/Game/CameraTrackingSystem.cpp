// GLM
#include <glm/gtc/matrix_transform.hpp>

// Game
#include <Game/CameraTrackingSystem.hpp>
#include <Game/PhysicsSystem.hpp>
#include <Game/PhysicsComponent.hpp>
#include <Game/World.hpp>

namespace Game {
	CameraTrackingSystem::CameraTrackingSystem(SystemArg arg) : System{arg} {
		static_assert(World::orderAfter<CameraTrackingSystem, CharacterMovementSystem>());
		static_assert(World::orderAfter<CameraTrackingSystem, PhysicsSystem>());
	}

	void CameraTrackingSystem::setup(Engine::Camera& camera) {
		this->camera = &camera;
	}

	void CameraTrackingSystem::run(float dt) {
		const auto focusPos = world.getComponent<PhysicsComponent>(focus).body->GetPosition();
		camera->setPosition(glm::vec2{focusPos.x, focusPos.y});
	}
}
