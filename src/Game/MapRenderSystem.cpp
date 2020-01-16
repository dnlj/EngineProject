// STD
#include <algorithm>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Game
#include <Game/MapSystem.hpp>
#include <Game/MapRenderSystem.hpp>


namespace Game {
	MapRenderSystem::MapRenderSystem(World& world) : SystemBase{world} {
		priorityAfter = world.getBitsetForSystems<Game::MapSystem>();
	}

	MapRenderSystem::~MapRenderSystem() {
		// TODO: loop over and delete render data
		// TODO: cleanup any other gl objects
	}

	
	void MapRenderSystem::setup(Engine::EngineInstance& engine) {
		camera = &engine.camera;
		shader = engine.shaderManager.get("shaders/terrain_v2");
		texture = engine.textureManager.get("../assets/test.png");
	}

	void MapRenderSystem::run(float dt) {
		glUseProgram(shader.get());

		// Setup Texture
		glBindTextureUnit(0, texture.get());
		glUniform1i(5, 0);

		const auto mvp = camera->getProjection() * camera->getView();

		//const auto& mapSys = world.getSystem<MapSystem>();
		//const auto screenBuffer = glm::ivec2{2, 2};
		//const auto screenBounds = camera->getWorldScreenBounds();
		//const auto minChunk = mapSys.blockToChunk(mapSys.worldToBlock(screenBounds.min)) - screenBuffer;
		//const auto maxChunk = mapSys.blockToChunk(mapSys.worldToBlock(screenBounds.max)) + screenBuffer;
		//
		//for (int x = minChunk.x; x <= maxChunk.x; ++x) {
		//	for (int y = minChunk.y; y <= maxChunk.y; ++y) {
		//		// If 
		//		//const auto index = chunkToIndex({x, y});
		//		//auto& data = chunkRenderData[index.x][index.y];
		//	}
		//}

		// TODO: I think something is wrong with the greedy expand

		/**
		 * For chunks in active area:
		 *     If chunk not in cache or chunk updated:
		 *         update chunk cache
		 *     draw chunk
		 */

		for (int x = 0; x < activeArea.x; ++x) {
			for (int y = 0; y < activeArea.y; ++y) {
				auto& data = chunkRenderData[x][y];
				if (data.updated) {
					data.updated = false;
					updateChunkRenderData(data);
				}

				drawChunk(data, mvp);
			}
		}
	}

	void MapRenderSystem::updateChunk(const glm::ivec2 chunkPos, const MapChunk* chunk) {
		constexpr auto chunkSize = glm::vec2{MapChunk::size} * MapChunk::tileSize;
		constexpr auto maxActiveArea = glm::vec2{activeArea / 2} * chunkSize;
		constexpr auto minActiveArea = -maxActiveArea;

		const auto& pos = chunk->getBody().GetPosition();

		if (pos.x < minActiveArea.x || pos.y < minActiveArea.y) { return; }
		if (pos.x > maxActiveArea.x || pos.y > maxActiveArea.y) { return; }

		const auto index = chunkToIndex(chunkPos);
		auto& data = chunkRenderData[index.x][index.y];
		data.updated = true;
		data.chunk = chunk;
	}
	
	glm::ivec2 MapRenderSystem::chunkToIndex(const glm::ivec2 chunk) const {
		return (activeArea + chunk % activeArea) % activeArea;
	}

	void MapRenderSystem::drawChunk(const RenderData& data, glm::mat4 mvp) const {
		const auto pos = data.chunk->getBody().GetPosition();
		mvp = glm::translate(mvp, glm::vec3(pos.x, pos.y, 0.0f));

		glBindVertexArray(data.vao);
		glUniformMatrix4fv(1, 1, GL_FALSE, &mvp[0][0]);
		glDrawElements(GL_TRIANGLES, data.elementCount, GL_UNSIGNED_SHORT, 0);
	}

	void MapRenderSystem::updateChunkRenderData(RenderData& data) {
		struct Vertex {
			glm::vec2 pos;
			GLuint texture = 0; // TODO: probably doesnt need to be 32bit
		};

		bool used[MapChunk::size.x][MapChunk::size.y] = {};
		// TODO: make `StaticVector`s?
		std::vector<Vertex> vboData;
		std::vector<GLushort> eboData;

		// TODO: Reserve vectors

		const auto usable = [&](const glm::ivec2 pos, const int blockType) {
			return !used[pos.x][pos.y] && data.chunk->data[pos.x][pos.y] == blockType;
		};

		for (glm::ivec2 begin = {0, 0}; begin.x < MapChunk::size.x; ++begin.x) {
			for (begin.y = 0; begin.y < MapChunk::size.y; ++begin.y) {
				// Greedy expand
				const auto blockType = static_cast<GLuint>(data.chunk->data[begin.x][begin.y]);
				if (blockType == MapChunk::AIR.id || !usable(begin, blockType)) { continue; }
				auto end = begin;

				while (end.y < MapChunk::size.y && usable(end, blockType)) { ++end.y; }

				do {
					std::fill(&used[end.x][begin.y], &used[end.x][end.y], true);
					++end.x;
				} while (end.x < MapChunk::size.x && usable(end, blockType));

				// Add buffer data
				glm::vec2 origin = glm::vec2{begin} * MapChunk::tileSize;
				glm::vec2 size = glm::vec2{end - begin} * MapChunk::tileSize;
				const auto vertexCount = static_cast<GLushort>(vboData.size());

				vboData.push_back({origin, blockType});
				vboData.push_back({origin + glm::vec2{size.x, 0}, blockType});
				vboData.push_back({origin + size, blockType});
				vboData.push_back({origin + glm::vec2{0, size.y}, blockType});

				eboData.push_back(vertexCount + 0);
				eboData.push_back(vertexCount + 1);
				eboData.push_back(vertexCount + 2);
				eboData.push_back(vertexCount + 2);
				eboData.push_back(vertexCount + 3);
				eboData.push_back(vertexCount + 0);
			}
		}

		data.elementCount = static_cast<GLsizei>(eboData.size());
		constexpr GLuint bindingIndex = 0;

		// TODO: move this out into some static init location.
		if (data.vao == 0) {
			glCreateVertexArrays(1, &data.vao);

			glEnableVertexArrayAttrib(data.vao, 0);
			glVertexArrayAttribFormat(data.vao, 0, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
			glVertexArrayAttribBinding(data.vao, 0, bindingIndex);

			// TODO: texture attribute
		}

		glDeleteBuffers(2, data.buffers);
		glCreateBuffers(2, data.buffers);

		glNamedBufferData(data.ebo, sizeof(eboData[0]) * eboData.size(), eboData.data(), GL_STATIC_DRAW);
		glVertexArrayElementBuffer(data.vao, data.ebo);

		glNamedBufferData(data.vbo, sizeof(vboData[0]) * vboData.size(), vboData.data(), GL_STATIC_DRAW);
		glVertexArrayVertexBuffer(data.vao, bindingIndex, data.vbo, 0, sizeof(Vertex));
	}
}
