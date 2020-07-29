#pragma once

// Game
#include <Game/System.hpp>
#include <Game/EntityFilter.hpp>


namespace Game {
	class CharacterMovementSystem : public System {
		public:
			CharacterMovementSystem(SystemArg arg);
			void tick();

		private:
			EntityFilter& filter;
	};
}
