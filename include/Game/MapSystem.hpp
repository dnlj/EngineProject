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
			/**
			 * Converts from a world position to chunk coordinates.
			 * @param[in] pos The position in world space.
			 * @return The chunk coordinates.
			 */
			glm::ivec2 worldToChunk(glm::vec2 pos) const;

			// TODO: Doc
			glm::vec2 chunkToWorld(glm::ivec2 pos) const;

			/**
			 * Get the chunk at a position.
			 * @param[in] pos The position in chunk coordinates.
			 * @return The chunk at the position.
			 */
			MapChunk& getChunkAt(glm::ivec2 pos);

			// TODO: Doc
			void loadChunk(glm::ivec2 pos);

			/** The number of chunks in the map */
			constexpr static glm::ivec2 mapSize = {8, 8};

			/** Number of chunks before shifting the origin */
			constexpr static int originRange = 2;

			/** Offset of current origin in increments of originRange */
			glm::ivec2 mapOffset = {0, 0};

			MapChunk chunks[mapSize.x][mapSize.y]{};

			Engine::InputManager* input;
			const Engine::Camera* camera;
			Engine::Shader shader;
			Engine::Texture texture;
	};
}
