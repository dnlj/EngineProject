#pragma once

// Engine
#include <Engine/Noise/WorleyNoise.hpp>
#include <Engine/Noise/OpenSimplexNoise.hpp>

// Game
#include <Game/Common.hpp>
#include <Game/BlockMeta.hpp>


namespace Game {
	class MapChunk; // Forward decl

	/**
	 * Potential biomes
	 * - Forest
	 * - Jungle
	 * - Taiga
	 * - Desert
	 * - Savanna
	 * - Ocean
	 * 
	 * ================================
	 * = Biome desc data considerations
	 * ================================
	 * Name
	 * 
	 * Relative size
	 * - Spawn shape? This would be viable if we switch to grid based biomes.
	 *   Biomes would be chosen on a cell basis based on size and then queried on a block basis if it is really in that biome
	 * 
	 * Valid spawn location - height, dist from spawn, only left, only right, etc
	 *
	 * Resources and their spawn conditions
	 * - This should be specified by biome and not resource because we
	 *	 may want the same resource in multiple biomes but with diff freq for example
	 *
	 * Landmarks and their spawn conditions
	 * - Same reason as resources
	 *
	 * Basis function for generic land shape and blending between biomes
	 * 
	 * 
	 */

	/**
	 * 
	 */
	class MapGenerator2 {
		public:
			using Float = float32;
			using Int = int32;
			using FVec2 = glm::vec<2, Float>;
			using IVec2 = glm::vec<2, Int>;

			enum class Biome {
				Default,
				Forest,
				Jungle,
				Taiga,
				Desert,
				Savanna,
				Ocean,
			};

			enum class Landmark {
				BossPortal,
				TreeDefault,
				TreeForest,
				TreeJungle,
			};

			template<Biome B>
			constexpr static auto landmarksByBiome = []{static_assert(B!=B, "No specialization for biome found.");}();
			
			template<> constexpr static std::array landmarksByBiome<Biome::Default> = {Landmark::BossPortal, Landmark::TreeDefault};
			template<> constexpr static std::array landmarksByBiome<Biome::Forest> = {Landmark::BossPortal, Landmark::TreeForest};
			template<> constexpr static std::array landmarksByBiome<Biome::Jungle> = {Landmark::BossPortal};

		private:
			// TODO: add impl that takes perm array ref instead of seed so we can share
			Engine::Noise::OpenSimplexNoise simplex;
			Engine::Noise::WorleyNoise worley;
			Engine::Noise::RangePermutation<256> perm;

			struct BiomeBounds {
				Int depth;
				IVec2 cell;
			};

			struct LandmarkSample {
				bool exists;
				Float basis;
				BlockId block;
			};

			/** Variance in underlying terrain height */
			constexpr static Float heightVar = 5000.0; // 
			constexpr static Float biomeScales[] = { // Must be divisible by the previous depth
				// We want to use divisions by three so that each biome can potentially spawn
				// at surface level. If we use two then only the first depth will be at surface
				// level and all others will be above or below it.
				9000,
				3000,
				1000,
			};

			constexpr static auto biomeScalesInv = []{
				std::array<Float, std::size(biomeScales)> inv;
				for (Int i = 0; const auto s : biomeScales) {
					inv[i++] = 1 / s;
				}
				return inv;
			}();

			// Offset used so that biomes are roughly centered at ground level
			constexpr static glm::vec2 biomeOffset = {0,
				biomeScales[0] / 2.0 + 200.0 // Experimentally terrain surface is around 100-300
			};

		public:
			MapGenerator2(const int64 seed)
				: simplex{seed}
				, worley{seed}
				, perm{seed} {
			}

			BlockId value(const Int x, const Int y) const noexcept;


			/**
			 * @param pos The position of the chunk in block coordinates.
			 * @param chunk The chunk to store the data in.
			 */
			void init(const IVec2 pos, MapChunk& chunk) const noexcept;

		private:
			// TODO: doc stages
			
			template<Biome B>
			ENGINE_INLINE BlockId calc(const IVec2 ipos, const FVec2 pos, const Float h0, const FVec2 posBiome, const BiomeBounds bounds) const noexcept;
			
			[[nodiscard]]
			BlockId resource(const FVec2 pos) const noexcept;
			
			[[nodiscard]]
			ENGINE_INLINE BiomeBounds biomeAt(const FVec2 pos) const noexcept;

			[[nodiscard]]
			ENGINE_INLINE Float height0(const Float x) const noexcept;
			
			template<Biome B>
			[[nodiscard]]
			int32 height(const Float x, const BiomeBounds bounds, const Float h0) const noexcept;
			
			template<Biome B>
			[[nodiscard]]
			ENGINE_INLINE LandmarkSample landmark(const FVec2 pos, const IVec2 ipos, const Int h) const noexcept;

			template<Biome B>
			[[nodiscard]]
			ENGINE_INLINE Float basisStrength(const FVec2 pos, const FVec2 posBiome, const BiomeBounds bounds) const noexcept;

			template<Biome B>
			[[nodiscard]]
			ENGINE_INLINE Float basis(const FVec2 pos, const Int h, const Float bstr) const noexcept;

			template<Biome B>
			[[nodiscard]]
			ENGINE_INLINE BlockId block(const FVec2 pos, const FVec2 ipos, const Int h, const Float h0, const BiomeBounds bounds, const Float bstr) const noexcept;

			/*
			[[nodiscard]]
			Int biome(const FVec2 pos) const noexcept;*/
			
			////////////////////////////////////////////////////////////////////////////////
			// Landmark specialization functions
			////////////////////////////////////////////////////////////////////////////////
			
			template<Landmark L>
			[[nodiscard]]
			ENGINE_INLINE LandmarkSample landmarkSample(const FVec2 pos, const IVec2 ipos, const Int h) const noexcept {
				static_assert(L != L, "Missing specialization.");
			}

			////////////////////////////////////////////////////////////////////////////////
			// Biome specialization functions
			////////////////////////////////////////////////////////////////////////////////
			
			[[nodiscard]]
			ENGINE_INLINE Float genericBiomeHeightStrength(const Float x, const BiomeBounds bounds) const noexcept;

			[[nodiscard]]
			ENGINE_INLINE Float genericBiomeBasisStrength(const FVec2 posBiome, const BiomeBounds bounds) const noexcept;

			[[nodiscard]]
			ENGINE_INLINE Float genericBiomeBlockStrength(const FVec2 pos, const Float basisStrength) const noexcept;

			// TODO: Doc
			template<Biome B>
			[[nodiscard]]
			ENGINE_INLINE Float biomeHeightOffset(const Float x) const noexcept {
				static_assert(B != B, "Missing specialization for biome.");
			}

			// TODO: Doc - range ~[0, 1]
			template<Biome B>
			[[nodiscard]]
			ENGINE_INLINE Float biomeHeightStrength(const Float x, const BiomeBounds bounds) const noexcept {
				static_assert(B != B, "Missing specialization for biome.");
			}

			// TODO: Doc - range ~[-1, 1]
			template<Biome B>
			[[nodiscard]]
			ENGINE_INLINE Float biomeBasis(const FVec2 pos, const int32 h) const noexcept {
				static_assert(B != B, "Missing specialization for biome.");
			}

			// TODO: Doc - range ~[0, 1]
			template<Biome B>
			[[nodiscard]]
			ENGINE_INLINE Float biomeBasisStrength(const FVec2 pos, const FVec2 posBiome, const BiomeBounds bounds) const noexcept {
				static_assert(B != B, "Missing specialization for biome.");
			}

			// TODO: Doc
			template<Biome B>
			[[nodiscard]]
			ENGINE_INLINE BlockId biomeBlock(const FVec2 pos, const IVec2 ipos, const Int h, const Float h0, const BiomeBounds bounds) const noexcept {
				static_assert(B != B, "Missing specialization for biome.");
			}

			// TODO: Doc - range ~[0, 1]
			template<Biome B>
			[[nodiscard]]
			ENGINE_INLINE Float biomeBlockStrength(const FVec2 pos, const Float basisStrength) const noexcept {
				static_assert(B != B, "Missing specialization for biome.");
			}
	};
}
