#pragma once

// Game
#include <Game/Common.hpp>


namespace Game {
	class CharacterMovementSystem : public SystemBase {
		public:
			CharacterMovementSystem(World& world);
			virtual void run(float dt) override;

		private:
			Engine::ECS::EntityFilter& filter;
	};
}
