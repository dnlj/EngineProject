#pragma once

// Game
#include <Game/BlockEntityData.hpp>


namespace Game {
	class BlockEntityComponent {
		public:
			glm::ivec2 block;
			BlockEntityType type;
	};
}
