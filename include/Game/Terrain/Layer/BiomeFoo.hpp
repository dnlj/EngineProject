#pragma once

// Game
#include <Game/Terrain/temp.hpp> // TODO: remove once everything is cleaned up.
#include <Game/Terrain/Layer/BiomeDebug.hpp>


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

	class BiomeFooStructureInfo: public Layer::DependsOn<> {
		public:
			using Range = Layer::ChunkArea;

		public:
			void request(const Range area, TestGenerator& generator);
			ENGINE_INLINE void generate(const Range area, TestGenerator& generator) {}; // No generation.
			void get(BIOME_STRUCTURE_INFO_ARGS) const noexcept;

		private:
			Engine::Noise::OpenSimplexNoise simplex{1234}; // TODO: Shared data/noise at the Generator level.
	};

	// TODO: Doc what layers biomes can have somewhere.
	class BiomeFoo {
		public:
			using Height = BiomeFooHeight;
			using BasisStrength = BiomeFooBasisStrength;
			using Basis = BiomeFooBasis;
			using Block = BiomeDebugBlock<BlockId::Debug, 1>;
			using StructureInfo = BiomeFooStructureInfo;
	};
}
