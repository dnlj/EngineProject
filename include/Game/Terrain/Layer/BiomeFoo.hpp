#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.


namespace Game::Terrain::Layer {
	class WorldBaseHeight;
	class BiomeHeight;

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

	class BiomeFooBasisStrength : public Layer::DependsOn<WorldBaseHeight> {
		public:
			using Range = Layer::ChunkArea;

		public:
			void request(const Range area, TestGenerator& generator);
			ENGINE_INLINE void generate(const Range area, TestGenerator& generator) {}; // No generation.
			Float get(BIOME_BASIS_STRENGTH_ARGS) const noexcept;

		private:
			Engine::Noise::OpenSimplexNoise simplex{1234}; // TODO: Shared data/noise at the Generator level.
	};

	// TODO: remove SimpleBiome
	class BiomeFooBasis: public Layer::DependsOn<WorldBaseHeight> {
		public:
			using Range = Layer::ChunkArea;

		public:
			void request(const Range area, TestGenerator& generator);
			ENGINE_INLINE void generate(const Range area, TestGenerator& generator) {}; // No generation.
			Float get(BIOME_BASIS_ARGS) const noexcept;

		private:
			Engine::Noise::OpenSimplexNoise simplex{1234}; // TODO: Shared data/noise at the Generator level.
	};

	class BiomeFoo {
		public:
			using Height = BiomeFooHeight;
			using BasisStrength = BiomeFooBasisStrength;
			using Basis = BiomeFooBasis;
	};
}
