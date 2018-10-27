#pragma once

// Engine
#include <Engine/EngineInstance.hpp>

// Game
#include <Game/MapChunk.hpp>

namespace Game {
	class Map {
		private:
			constexpr static int chunkCountX = 4;
			constexpr static int chunkCountY = chunkCountX;

			MapChunk chunks[chunkCountX][chunkCountY]{};

		public:
			void setup(World& world);
			void update(Engine::EngineInstance& engine, World& world);
	};
}
