#pragma once

// Game
#include <Game/Common.hpp>

namespace Game {
	using BlockId = int16;
	class MapBlock {
		public:
			const BlockId id;
			const bool solid;
	};
}
