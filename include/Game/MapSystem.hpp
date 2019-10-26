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

			// TODO: Doc
			template<MapChunk::EditMemberFunction func>
			void applyEdit();

		private:
			/**
			 * Converts from a world position to chunk coordinates.
			 * @param[in] pos The position in world space.
			 * @return The chunk coordinates.
			 */
			glm::ivec2 worldToChunk(glm::vec2 pos) const;

			/**
			 * Converts from a chunk position to world coordinates.
			 * @param[in] pos The position in chunk space.
			 * @return The world coordinates.
			 */
			glm::vec2 chunkToWorld(glm::ivec2 pos) const;

			/**
			 * Get the chunk at a position.
			 * @param[in] pos The position in chunk coordinates.
			 * @return The chunk at the position.
			 */
			MapChunk& getChunkAt(glm::ivec2 pos);

			// TODO: Doc
			MapChunk& ensureChunkLoaded(glm::ivec2 pos);

			// TODO: Doc
			void loadChunk(MapChunk& chunk, glm::ivec2 pos);

			// TODO: Doc
			glm::ivec2 chunkToRegion(glm::ivec2 pos);

			// TODO: Doc
			glm::ivec2 regionToChunk(glm::ivec2 region);

			// TODO: Doc
			void loadRegion(const glm::ivec2 region);

			// TODO: Doc
			void updateOrigin();

			/** Number of chunks before shifting the origin */
			constexpr static int originRange = 4; // Absolute Range = originRange * chunkSize * tileSize

			/** Size of regions in chunks */
			constexpr static glm::ivec2 regionSize = {16, 16};

			/** The number of regions in the map*/
			constexpr static glm::ivec2 regionCount = {3, 3};

			/** Offset of current origin in increments of originRange */
			glm::ivec2 mapOffset = {0, 0};

			/** The number of chunks in the map */
			constexpr static glm::ivec2 mapSize = regionCount * regionSize;

			MapChunk chunks[mapSize.x][mapSize.y]{};

			Engine::InputManager* input;
			const Engine::Camera* camera;
			Engine::Shader shader;
			Engine::Texture texture;
	};
}

#include <Game/MapSystem.ipp>
