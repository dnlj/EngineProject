#pragma once

// Game
#include <Game/UI/AutoList.hpp>


namespace Game::UI {
	class CoordPane : public AutoList {
		public:
			enum {
				Camera,

				CursorPos,
				CursorWorldOffset,
				CursorWorldPos,

				MapOffset,
				MapOffsetBlock,
				MapOffsetChunk,

				MouseOffset,
				MouseWorld,
				MouseBlock,
				MouseBlockWorld,
				MouseChunk,
				MouseRegion,
			};

			CoordPane(EUI::Context* context);
	};
}
