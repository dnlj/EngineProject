// Game
#include <Game/InputSystem.hpp>
#include <Game/InputComponent.hpp>

namespace Game {
	InputSystem::InputSystem(World& world) : SystemBase{world} {
		cbits = world.getBitsetForComponents<InputComponent>();
	}

	void InputSystem::setup(Engine::InputManager& inputManager) {
		this->inputManager = &inputManager;
	}

	void InputSystem::onEntityAdded(Engine::ECS::EntityID eid) {
		world.getComponent<InputComponent>(eid).inputManager = inputManager;
	}

	void InputSystem::onEntityRemoved(Engine::ECS::EntityID eid) {
		world.getComponent<InputComponent>(eid).inputManager = nullptr;
	}

	void InputSystem::run(float dt) {
		inputManager->update();
	}
}
