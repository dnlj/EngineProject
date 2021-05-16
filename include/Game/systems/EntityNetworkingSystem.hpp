#pragma once

// Game
#include <Game/System.hpp>
#include <Game/comps/NeighborsComponent.hpp>


namespace Game {
	class EntityNetworkingSystem : public System {
		private:
			NeighborsComponent::Set lastNeighbors;

		public:
			using System::System;
			void tick();
	};
}
