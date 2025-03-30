#pragma once

#include <Game/Terrain/Generator.hpp>

namespace Game::Terrain {
	// TODO: Should have a way to detect cycles.
	template<class TerrainType>
	class LayerTesting {
		public:
			// TODO: depends on
			// TODO: should cache data?
			// TODO: should save data?
	
			// TODO: Units could be meters, blocks, chunks, etc.
			using Input = int;
			void request(const Input& input, TerrainType&);
			void requestImmediate(const Input& input, TerrainType&);
			void generate(const Input& input, TerrainType&);
			void get();
	};

	template<class... Layers>
	class Terrain2 {
		public:
			// TODO: does this need to be on the layer or terrain? Both really I think.
			template<class Layer>
			void request(const Layer::Input& input);
			//void requestImmediate(const Input& input);

		private:
			std::tuple<Layers...> layers;
			// TODO: store tuple of requests for each layer.
	};
}

namespace Game::Terrain::Layer {
	// TODO: incorperate this at the terrain level to verify the correct requests are made and avoid cycles.
	template<class...>
	class DependsOn{};
	class BiomeA;
	class BiomeB;
	class BiomeC;

	class ChunkArea {
		public:
			ChunkVec min;
			ChunkVec max;
	};

	// TODO: How should we split up generate calls? Sometimes it better to just pass in
	//       the whole input range, others only a subset or single entry. Do layers need
	//       to be able to define how requests are split (or not split)? Consider SIMD and
	//       multithreading cases. When you call Layer::request should it instead split
	//       out jobs? Probably not because we also want ot check for overlapping/subset
	//       request and remove those. So we may need a different mechanism for
	//       multiprocessing cases.
	//       - Should the input types themselves be able to describe how to be split?
	//       - How would we know how large of batches to create?
	// TODO: should layer be able to define their own optimize requestion function to
	//       determine how requests get combined?
	// TODO: Refactor return types to only return direct layer info once integrated.
	// Basis
	//   BiomeBlended
	//	 BiomeA
	//	 BiomeB
	//   BlendedHeight
	//     BiomeHeight
	//       WorldHeight
	//     BiomeBlended
	//       BiomeWeighted
	//         BiomeRaw
	//           WorldHeight

	// The large world-scale height variation that persists between all biomes.
	class WorldBaseHeight : public DependsOn<> {
		public:
			class Input {
				public:
					BlockUnit xMin;
					BlockUnit xMax;
			};

		public:
			void request(const Input& area);
			void generate(BlockUnit x);
			BlockUnit get(BlockUnit x);

		private:
			// TODO: Would like a way to ask it to clear cache per top level request/"batch" if possible.
	};

	// The direct, raw, biome info. Determines what biome is where before any blending.
	class BiomeRaw : public DependsOn<> {
		public:
			using Input = ChunkArea;

		public:
			void request(const Input& area);
			void generate(BlockVec blockCoord);
			BiomeRawInfo get(BlockVec blockCoord);
	};

	// The absolute weight of each biome. These are non-normalized.
	class BiomeWeights : public DependsOn<BiomeRaw> {
		public:
			using Input = ChunkArea;

		public:
			void request(const Input& area);
			void generate(BlockVec blockCoord);
			BiomeWeights get(BlockVec blockCoord);
	};

	// TODO: this will also depend on all biomes.
	// The total basis strength after blending all biomes.
	class BlendedBasisStrength : public DependsOn<BiomeWeights, BiomeA, BiomeB, BiomeC> {
		public:
			using Input = ChunkArea;

		public:
			void request(const Input& area);
			void generate(BlockVec blockCoord);
			BiomeBlend get(BlockVec blockCoord);
	};

	class BlendedBasis : public DependsOn<BlendedBasisStrength, BiomeA, BiomeB, BiomeC> {
		public:
			using Input = ChunkArea;

		public:
			void request(const Input& area);
			void generate(BlockVec blockCoord);
			BasisInfo get(BlockVec blockCoord);
	};
}
