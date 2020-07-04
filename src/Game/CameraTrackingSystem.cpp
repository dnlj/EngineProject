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
		, activePlayerFilter{world.getFilterFor<PlayerFlag, PhysicsComponent>()} {
		static_assert(World::orderAfter<CameraTrackingSystem, CharacterMovementSystem>());
		static_assert(World::orderAfter<CameraTrackingSystem, PhysicsSystem>());
		engine.camera.setPosition({0,0});
	}

	void CameraTrackingSystem::run(float dt) {
		for (auto ent : activePlayerFilter) {
			const auto focusPos = world.getComponent<PhysicsComponent>(ent).getInterpPosition();
			engine.camera.setPosition(glm::vec2{focusPos.x, focusPos.y});
		}
	}
}
