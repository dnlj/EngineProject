#pragma once

// STD
#include <type_traits>

// GLM
#include <glm/glm.hpp>

// Game
#include <Game/MapBlock.hpp>
#include <Game/PhysicsSystem.hpp>


// TODO: Doc
namespace Game {
	class MapChunk {
		public:
			constexpr static MapBlock AIR{0, false};
			constexpr static MapBlock DIRT{1, true};

		public:
			constexpr static glm::ivec2 size = {32, 32};
			constexpr static auto blockSize = 1.0f/6.0f;

		public:
			std::decay_t<decltype(MapBlock::id)> data[size.x][size.y] = {};
			glm::ivec2 pos = {0x7FFF'FFFF, 0x7FFF'FFFF};
			bool updated = false;
	};
}
