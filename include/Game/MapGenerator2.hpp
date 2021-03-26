#pragma once

// Engine
#include <Engine/Noise/WorleyNoise.hpp>
#include <Engine/Noise/OpenSimplexNoise.hpp>

// Game
#include <Game/Common.hpp>
#include <Game/BlockMeta.hpp>


namespace Game {
	class MapGenerator2 {
		private:
			Engine::Noise::OpenSimplexNoise simplex;
			Engine::Noise::WorleyNoise worley;

		public:
			MapGenerator2(const int64 seed) : simplex{seed}, worley{seed} {
			}

			BlockId value(const int32 x, const int32 y) const noexcept;

		private:
			BlockId resource(const glm::vec2 pos) const noexcept;
			float32 basis(const glm::vec2 pos) const noexcept;
	};
}
