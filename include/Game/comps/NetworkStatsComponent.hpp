#pragma once

// Game
#include <Game/Common.hpp>


namespace Game {
	class NetworkStatsComponent {
		public:
			int32 inputBufferSize = 0;
			float32 idealInputBufferSize = 0;
	};
}
