#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.


namespace Game::Terrain::Layer {
	class WorldBaseHeight;

	class BiomeFooHeight : public Layer::DependsOn<WorldBaseHeight> {
		public:
			using Range = Layer::ChunkSpanX;

		public:
			void request(const Range area, TestGenerator& generator);
			ENGINE_INLINE void generate(const Range area, TestGenerator& generator) {}; // No generation.
			Float get(BIOME_HEIGHT_ARGS) const noexcept;

		private:
			Engine::Noise::OpenSimplexNoise simplex{1234}; // TODO: Shared data/noise at the Generator level.
	};

	class BiomeFoo {
		public:
			using Height = BiomeFooHeight;
	};
}
