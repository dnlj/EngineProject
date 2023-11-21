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
				CursorWorldAbsPos,

				ZoneOffset,

				TargetOffset,
				TargetWorld,
				TargetBlock,
				TargetBlockWorld,
				TargetChunk,
				TargetRegion,
			};

			CoordPane(EUI::Context* context);
	};
}
