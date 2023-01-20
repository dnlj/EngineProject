#pragma once

// Game
#include <Game/Common.hpp>


namespace Game {
	class NetworkStatsComponent {
		public:
			int32 inputBufferSize = -1;
			float32 idealInputBufferSize = -1;
	};
}
