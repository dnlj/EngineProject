// GLM
#include <glm/gtc/matrix_transform.hpp>

// Game
#include <Game/systems/CameraTrackingSystem.hpp>
#include <Game/comps/PhysicsInterpComponent.hpp>
#include <Game/World.hpp>

namespace Game {
	CameraTrackingSystem::CameraTrackingSystem(SystemArg arg)
		: System{arg} {
		static_assert(World::orderAfter<CameraTrackingSystem, CharacterMovementSystem>());
		static_assert(World::orderAfter<CameraTrackingSystem, PhysicsSystem>());
		engine.getCamera().setPosition({0,0});
	}
	
	void CameraTrackingSystem::run(float dt) {
		auto& cam = engine.getCamera();
		for (auto ent : world.getFilter<CameraTargetFlag, PhysicsInterpComponent>()) {
			const auto focusPos = world.getComponent<PhysicsInterpComponent>(ent).getPosition();
			cam.setPosition(glm::vec2{focusPos.x, focusPos.y});
		}
	}
}
