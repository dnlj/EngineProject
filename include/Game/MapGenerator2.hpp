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


	class NoiseBase {
	};

	class Biome {
		public:
			/**
			 * @param nb The set of noise objects to use.
			 * @param pos The position to check.
			 * @param origin The bottom left position of the biome.
			 * @param size The size of the biome.
			 */
			[[nodiscard]]
			float32 strength(const NoiseBase& nb, const glm::ivec2 pos, glm::ivec2 origin, glm::ivec2 size) const noexcept {
			};
	};

	/**
	 * 
	 */
	class MapGenerator2 {
		private:
			// TODO: add impl that takes perm array ref instead of seed so we can share
			Engine::Noise::OpenSimplexNoise simplex;
			Engine::Noise::WorleyNoise worley;
			Engine::Noise::RangePermutation<256> perm;

			struct BiomeBounds {
				int depth;
				glm::ivec2 cell;
			};

			/** Variance in underlying terrain height */
			constexpr static float32 heightVar = 5000.0f; // 
			constexpr static float32 biomeScales[] = { // Must be divisible by the previous depth
				// We want to use divisions by three so that each biome can potentially spawn
				// at surface level. If we use two then only the first depth will be at surface
				// level and all others will be above or below it.
				9000,
				3000,
				1000,
			};

			constexpr static auto biomeScalesInv = []{
				std::array<float32, std::size(biomeScales)> inv;
				for (int i = 0; const auto s : biomeScales) {
					inv[i++] = 1.0f / s;
				}
				return inv;
			}();

			// Offset used so that biomes are roughly centered at ground level
			constexpr static glm::vec2 biomeOffset = {0,
				biomeScales[0] / 2.0f + 200 // Experimentally terrain surface is around 100-300
			};

		public:
			MapGenerator2(const int64 seed)
				: simplex{seed}
				, worley{seed}
				, perm{seed} {
			}

			BlockId value(const int32 x, const int32 y) const noexcept;

			/**
			 * @param pos The position of the chunk in block coordinates.
			 * @param chunk The chunk to store the data in.
			 */
			void init(const glm::ivec2 pos, MapChunk& chunk) const noexcept;

			void advance(const glm::ivec2 pos, MapChunk& chunk, const MapChunk& top, const MapChunk& right, const MapChunk& bottom, const MapChunk& left);

		private:
			[[nodiscard]]
			BlockId resource(const glm::vec2 pos) const noexcept;

			//[[nodiscard]]
			//int32 height(const glm::vec2 pos) const noexcept;
			
			[[nodiscard]]
			ENGINE_INLINE float32 height0(const float32 x) const noexcept;

			[[nodiscard]]
			int32 height(const float32 x, const BiomeBounds bounds) const noexcept;

			[[nodiscard]]
			BiomeBounds biomeAt(const glm::vec2 pos) const noexcept;

			[[nodiscard]]
			int32 biome(const glm::vec2 pos) const noexcept;

			// TODO: Doc
			template<int I>
			[[nodiscard]]
			ENGINE_INLINE float32 biomeHeightOffset(const float32 x) const noexcept {
				static_assert(I != I, "Missing specialization for biome.");
			}

			// TODO: Doc - range ~[0, 1]
			template<int I>
			[[nodiscard]]
			ENGINE_INLINE float32 biomeHeightStrength(const float32 x, const BiomeBounds bounds) const noexcept {
				static_assert(I != I, "Missing specialization for biome.");
			}

			// TODO: Doc - range ~[-1, 1]
			template<int I>
			[[nodiscard]]
			ENGINE_INLINE float32 biomeBasis(const glm::vec2 pos, const int32 h) const noexcept {
				static_assert(I != I, "Missing specialization for biome.");
			}

			// TODO: Doc - range ~[0, 1]
			template<int I>
			[[nodiscard]]
			ENGINE_INLINE float32 biomeBasisStrength(const glm::vec2 posAdj, const BiomeBounds bounds) const noexcept {
				static_assert(I != I, "Missing specialization for biome.");
			}

			// TODO: Doc - range ~[0, 1]
			template<int I>
			[[nodiscard]]
			ENGINE_INLINE BlockId biomeBlock(const glm::vec2 pos, const int32 h, const BiomeBounds bounds) const noexcept {
				static_assert(I != I, "Missing specialization for biome.");
			}

			// TODO: Doc - range ~[0, 1]
			template<int I>
			[[nodiscard]]
			ENGINE_INLINE float32 biomeBlockStrength(const float32 str) const noexcept {
				static_assert(I != I, "Missing specialization for biome.");
			}
	};
}
