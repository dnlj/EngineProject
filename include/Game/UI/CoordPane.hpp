#pragma once

// Game
#include <Game/UI/AutoList.hpp>


namespace Game::UI {
	class CoordPane : public AutoList {
		public:
			enum {
				MouseOffset,
				MouseWorld,
				MouseBlock,
				MouseBlockWorld,
				MouseChunk,
				MouseRegion,
				Camera,
				MapOffset,
				MapOffsetBlock,
				MapOffsetChunk,
			};

			CoordPane(EUI::Context* context);
	};
}
