// Game
#include <Game/InputSystem.hpp>
#include <Game/InputComponent.hpp>

namespace Game {
	InputSystem::InputSystem(World& world) : SystemBase{world} {
		// TODO: Fix
		//cbits = world.getBitsetForComponents<InputComponent>();
		priorityBefore.set();
		priorityBefore.reset(world.getSystemID<InputSystem>());
	}

	void InputSystem::setup(Engine::InputManager& inputManager) {
		this->inputManager = &inputManager;
	}

	// TODO: Fix
	//void InputSystem::onEntityAdded(Engine::ECS::Entity ent) {
	//	world.getComponent<InputComponent>(ent).inputManager = inputManager;
	//}
	//
	//void InputSystem::onEntityRemoved(Engine::ECS::Entity ent) {
	//	world.getComponent<InputComponent>(ent).inputManager = nullptr;
	//}

	void InputSystem::run(float dt) {
		inputManager->update();
	}
}
