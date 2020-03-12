// GLM
#include <glm/gtc/matrix_transform.hpp>

// Game
#include <Game/CameraTrackingSystem.hpp>
#include <Game/PhysicsSystem.hpp>
#include <Game/PhysicsComponent.hpp>
#include <Game/World.hpp>

namespace Game {
	CameraTrackingSystem::CameraTrackingSystem(SystemArg arg)
		: System{arg} {
		static_assert(World::orderAfter<CameraTrackingSystem, CharacterMovementSystem>());
		static_assert(World::orderAfter<CameraTrackingSystem, PhysicsSystem>());
	}

	void CameraTrackingSystem::run(float dt) {
		// TODO: this should be in tick? no point in updating if objects havent moved.
		const auto focusPos = world.getComponent<PhysicsComponent>(focus).body->GetPosition();
		engine.camera.setPosition(glm::vec2{focusPos.x, focusPos.y});
	}
}
