#pragma once

// Engine
#include <Engine/EngineInstance.hpp>

// Game
#include <Game/MapChunk.hpp>

namespace Game {
	class MapSystem : public SystemBase {
		public:
			MapSystem(World& world);
			void setup(Engine::EngineInstance& engine);
			void run(float dt) override;

		private:
			constexpr static int chunkCountX = 4;
			constexpr static int chunkCountY = chunkCountX;

			MapChunk chunks[chunkCountX][chunkCountY]{};

			Engine::InputManager* input;
			const Engine::Camera* camera;
	};
}
