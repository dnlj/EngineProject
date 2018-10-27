#pragma once


// Game
#include <Game/Common.hpp>


namespace Game {
	class MapRenderSystem : public SystemBase {
		public:
			MapRenderSystem(World& world);
			void run(float dt) override;
	};
}
