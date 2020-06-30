#pragma once

// Game
#include <Game/System.hpp>
#include <Game/EntityFilter.hpp>
#include <Game/NeighborsComponent.hpp>


namespace Game {
	class NeighborSystem : public System {
		private:
			EntityFilter& neighborFilter;
			NeighborsComponent::Set last;

		public:
			NeighborSystem(SystemArg arg);
			void tick(float32 dt);
	};
}
