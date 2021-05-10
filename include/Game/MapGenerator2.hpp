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
				int depth;
				glm::ivec2 cell;
			};

			struct LandmarkSample {
				bool exists;
				float32 basis;
				BlockId block;
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
			// TODO: doc stages
			
			template<Biome B>
			ENGINE_INLINE BlockId calc(const glm::ivec2 ipos, const glm::vec2 pos, const float32 h0, const glm::vec2 posBiome, const BiomeBounds bounds) const noexcept;
			
			[[nodiscard]]
			BlockId resource(const glm::vec2 pos) const noexcept;
			
			[[nodiscard]]
			ENGINE_INLINE BiomeBounds biomeAt(const glm::vec2 pos) const noexcept;

			[[nodiscard]]
			ENGINE_INLINE float32 height0(const float32 x) const noexcept;
			
			template<Biome B>
			[[nodiscard]]
			int32 height(const float32 x, const BiomeBounds bounds, const float32 h0) const noexcept;
			
			template<Biome B>
			[[nodiscard]]
			ENGINE_INLINE LandmarkSample landmark2(const glm::vec2 pos, const glm::ivec2 ipos, const int32 h) const noexcept;

			[[nodiscard]]
			ENGINE_INLINE BlockId landmark(const glm::vec2 pos, const glm::ivec2 ipos, const int32 h) const noexcept;
			
			template<Biome B>
			[[nodiscard]]
			ENGINE_INLINE float32 basisStrength(const glm::vec2 pos, const glm::vec2 posBiome, const BiomeBounds bounds) const noexcept;

			template<Biome B>
			[[nodiscard]]
			ENGINE_INLINE float32 basis(const glm::vec2 pos, const int32 h, const float32 bstr) const noexcept;

			template<Biome B>
			[[nodiscard]]
			ENGINE_INLINE BlockId block(const glm::vec2 pos, const glm::vec2 ipos, const int32 h, const float32 h0, const BiomeBounds bounds, const float32 bstr) const noexcept;

			/*
			[[nodiscard]]
			int32 biome(const glm::vec2 pos) const noexcept;*/
			
			////////////////////////////////////////////////////////////////////////////////
			// Landmark specialization functions
			////////////////////////////////////////////////////////////////////////////////
			
			template<Landmark L>
			[[nodiscard]]
			ENGINE_INLINE LandmarkSample landmarkBasis(const glm::vec2 pos, const glm::ivec2 ipos) const noexcept {
				static_assert(L != L, "Missing specialization.");
			}

			template<Landmark L>
			[[nodiscard]]
			ENGINE_INLINE BlockId landmarkBlock(const glm::vec2 pos, const glm::ivec2 ipos, const int32 h) const noexcept {
				static_assert(L != L, "Missing specialization.");
			}

			////////////////////////////////////////////////////////////////////////////////
			// Biome specialization functions
			////////////////////////////////////////////////////////////////////////////////

			// TODO: Doc
			template<Biome B>
			[[nodiscard]]
			ENGINE_INLINE float32 biomeHeightOffset(const float32 x) const noexcept {
				static_assert(B != B, "Missing specialization for biome.");
			}

			// TODO: Doc - range ~[0, 1]
			template<Biome B>
			[[nodiscard]]
			ENGINE_INLINE float32 biomeHeightStrength(const float32 x, const BiomeBounds bounds) const noexcept {
				static_assert(B != B, "Missing specialization for biome.");
			}

			// TODO: Doc - range ~[-1, 1]
			template<Biome B>
			[[nodiscard]]
			ENGINE_INLINE float32 biomeBasis(const glm::vec2 pos, const int32 h) const noexcept {
				static_assert(B != B, "Missing specialization for biome.");
			}

			// TODO: Doc - range ~[0, 1]
			template<Biome B>
			[[nodiscard]]
			ENGINE_INLINE float32 biomeBasisStrength(const glm::vec2 pos, const glm::vec2 posBiome, const BiomeBounds bounds) const noexcept {
				static_assert(B != B, "Missing specialization for biome.");
			}

			// TODO: Doc
			template<Biome B>
			[[nodiscard]]
			ENGINE_INLINE BlockId biomeBlock(const glm::vec2 pos, const glm::ivec2 ipos, const int32 h, const float32 h0, const BiomeBounds bounds) const noexcept {
				static_assert(B != B, "Missing specialization for biome.");
			}

			// TODO: Doc - range ~[0, 1]
			template<Biome B>
			[[nodiscard]]
			ENGINE_INLINE float32 biomeBlockStrength(const float32 basisStrength) const noexcept {
				static_assert(B != B, "Missing specialization for biome.");
			}
	};
}
