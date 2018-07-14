#pragma once

// Engine
#include <Engine/SystemBase.hpp>
#include <Engine/InputManager.hpp>

// Game
#include <Game/Common.hpp>


namespace Game {
	class InputSystem : public SystemBase {
		public:
			InputSystem(World& world);
			void setup(Engine::InputManager& inputManager);

			virtual void onEntityAdded(Engine::ECS::Entity ent) override;
			virtual void onEntityRemoved(Engine::ECS::Entity ent) override;
			virtual void run(float dt) override;

		private:
			Engine::InputManager* inputManager;
	};
}
