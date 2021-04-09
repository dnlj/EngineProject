#pragma once

// Engine
#include <Engine/Noise/WorleyNoise.hpp>
#include <Engine/Noise/OpenSimplexNoise.hpp>

// Game
#include <Game/Common.hpp>
#include <Game/BlockMeta.hpp>


namespace Game {
	class MapChunk; // Forward decl

	class MapGenerator2 {
		private:
			// TODO: add impl that takes perm array ref instead of seed so we can share
			Engine::Noise::OpenSimplexNoise simplex;
			Engine::Noise::WorleyNoise2 worley;
			Engine::Noise::RangePermutation<256> treePerm;

		public:
			MapGenerator2(const int64 seed)
				: simplex{seed}
				, worley{seed}
				, treePerm{seed} {
			}

			BlockId value(const int32 x, const int32 y) const noexcept;

			/**
			 * @param pos The position of the chunk in block coordinates.
			 * @param chunk The chunk to store the data in.
			 */
			void init(const glm::ivec2 pos, MapChunk& chunk) const noexcept;

			void advance(const glm::ivec2 pos, MapChunk& chunk, const MapChunk& top, const MapChunk& right, const MapChunk& bottom, const MapChunk& left);

		private:
			BlockId resource(const glm::vec2 pos) const noexcept;
			int32 height(const glm::vec2 pos) const noexcept;
	};
}
