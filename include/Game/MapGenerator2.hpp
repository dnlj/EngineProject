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
	 * - Tiaga
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
		private:
			// TODO: add impl that takes perm array ref instead of seed so we can share
			Engine::Noise::OpenSimplexNoise simplex;
			Engine::Noise::WorleyNoise worley;
			Engine::Noise::RangePermutation<256> perm;

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

			[[nodiscard]]
			int32 height(const glm::vec2 pos) const noexcept;
			
			[[nodiscard]]
			BlockId biome2(const glm::vec2 pos) const noexcept;

			[[nodiscard]]
			int32 biome(const glm::vec2 pos) const noexcept;
	};
}
