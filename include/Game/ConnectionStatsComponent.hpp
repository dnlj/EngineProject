#pragma once

// Game
#include <Game/Common.hpp>

namespace Game {
	class ConnectionStatsComponent {
		public:
			constexpr static int32 seconds = 8;
			constexpr static int32 points = tickrate * seconds;
			float32 bytesSentTotal[points] = {};
			float32 bytesSent[points] = {};
	};
}
