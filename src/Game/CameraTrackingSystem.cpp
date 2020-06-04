// GLM
#include <glm/gtc/matrix_transform.hpp>

// Game
#include <Game/CameraTrackingSystem.hpp>
#include <Game/PhysicsSystem.hpp>
#include <Game/PhysicsComponent.hpp>
#include <Game/World.hpp>

namespace Game {
	CameraTrackingSystem::CameraTrackingSystem(SystemArg arg)
		: System{arg}
		, activePlayerFilter{world.getFilterFor<Game::ActivePlayerFlag>()} {
		static_assert(World::orderAfter<CameraTrackingSystem, CharacterMovementSystem>());
		static_assert(World::orderAfter<CameraTrackingSystem, PhysicsSystem>());
	}

	void CameraTrackingSystem::run(float dt) {
		for (auto ent : activePlayerFilter) {
			const auto focusPos = world.getComponent<PhysicsComponent>(ent).getInterpPosition();
			engine.camera.setPosition(glm::vec2{focusPos.x, focusPos.y});
		}
	}
}
