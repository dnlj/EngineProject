#pragma once

// GLM
#include <glm/vec2.hpp>

// Engine
#include <Engine/EngineInstance.hpp>
#include <Engine/ShaderManager.hpp>

// Game
#include <Game/MapChunk.hpp>

namespace Game {
	class MapSystem : public SystemBase {
		public:
			MapSystem(World& world);
			void setup(Engine::EngineInstance& engine);
			void run(float dt) override;
			const glm::ivec2& getOffset() const;

		private:
			constexpr static int chunkCountX = 2;
			constexpr static int chunkCountY = chunkCountX;
			constexpr static int originRange = 4; // TODO: Change range to chunks?

			glm::ivec2 mapOffset = {0, 0};

			MapChunk chunks[chunkCountX][chunkCountY]{};

			Engine::InputManager* input;
			const Engine::Camera* camera;
			Engine::Shader shader;
			Engine::Texture texture;
	};
}
