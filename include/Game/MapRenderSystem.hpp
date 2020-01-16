#pragma once

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// Engine
#include <Engine/EngineInstance.hpp>
#include <Engine/TextureManager.hpp>
#include <Engine/ShaderManager.hpp>

// Game
#include <Game/Common.hpp>
#include <Game/MapChunk.hpp>


// TODO: Doc
namespace Game {
	class MapRenderSystem : public SystemBase {
		public:
			MapRenderSystem(World& world);
			~MapRenderSystem();

			void run(float dt);

			void setup(Engine::EngineInstance& engine);

			void updateChunk(const glm::ivec2 chunkPos, const MapChunk* chunk);

		private:
			// TODO: All chunks could share one vbo and just update ebo
			// TODO: Pull block texture from separate buffer? Can we set this up in vao?
			// TODO: look into array textures
			struct RenderData {
				bool updated = false;
				const MapChunk* chunk;
				GLuint vao = 0;
				GLsizei elementCount = 0;

				union {
					GLuint buffers[2] = {0, 0};
					struct {
						GLuint vbo;
						GLuint ebo;
					};
				};
			};

			const Engine::Camera* camera;
			Engine::Shader shader;
			Engine::Texture texture;

			// TODO: rename. This is the size of our chunk cache. not
			// TODO: this should probably be on the mapsystem and should be used for chunk loading around the screen area
			// TODO: this really should be based on screen size/view area
			/** The number of chunks in the active game area */
			constexpr static glm::ivec2 activeArea = {8, 8};

			// TODO: rename cachedRenderData or similar
			RenderData chunkRenderData[activeArea.x][activeArea.y];

			glm::ivec2 chunkToIndex(const glm::ivec2 chunk) const;

			void drawChunk(const RenderData& data, glm::mat4 mvp) const;

			void updateChunkRenderData(RenderData& data);
	};
}
