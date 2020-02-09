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
		for (int x = 0; x < activeArea.x; ++x) {
			for (int y = 0; y < activeArea.y; ++y) {
				auto& data = chunkRenderData[x][y];
				glDeleteVertexArrays(1, &data.vao);
				glDeleteBuffers(data.numBuffers, data.buffers);
			}
		}
	}

	
	void MapRenderSystem::setup(Engine::EngineInstance& engine) {
		camera = &engine.camera;
		shader = engine.shaderManager.get("shaders/terrain_v2");
		texture = engine.textureManager.get("../assets/test.png");

		const auto& mapSys = world.getSystem<MapSystem>();
		for (int x = 0; x < activeArea.x; ++x) {
			for (int y = 0; y < activeArea.y; ++y) {
				auto& data = chunkRenderData[x][y];
				data.chunk = &mapSys.getChunkAt({x, y});
				data.updated = true;
				
				glCreateVertexArrays(1, &data.vao);
				
				glEnableVertexArrayAttrib(data.vao, positionAttribLocation);
				glVertexArrayAttribFormat(data.vao, positionAttribLocation, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
				glVertexArrayAttribBinding(data.vao, positionAttribLocation, bufferBindingIndex);

				// TODO: texture attribute
			}
		}
	}

	void MapRenderSystem::run(float dt) {
		glUseProgram(shader.get());

		// Setup Texture
		glBindTextureUnit(0, texture.get());
		glUniform1i(5, 0);

		const auto vp = camera->getProjection() * camera->getView();

		const auto& mapSys = world.getSystem<MapSystem>();
		constexpr auto screenBuffer = glm::ivec2{0, 0}; // TODO: this only makes sense if we are multithreading
		const auto screenBounds = camera->getWorldScreenBounds();
		const auto minChunk = mapSys.blockToChunk(mapSys.worldToBlock(screenBounds.min)) - screenBuffer;
		const auto maxChunk = mapSys.blockToChunk(mapSys.worldToBlock(screenBounds.max)) + screenBuffer;
		
		for (glm::ivec2 chunk = minChunk; chunk.x <= maxChunk.x; ++chunk.x) {
			for (chunk.y = minChunk.y; chunk.y <= maxChunk.y; ++chunk.y) {
				const auto index = chunkToIndex(chunk);
				auto& data = chunkRenderData[index.x][index.y];

				if (data.chunk->getPosition() != chunk) {
					data.chunk = &mapSys.getChunkAt(chunk);
					data.updated = true;
				}

				if (data.updated) {
					updateChunkRenderData(data);
				}

				const auto pos = data.chunk->getBody().GetPosition();
				const auto mvp = glm::translate(vp, glm::vec3(pos.x, pos.y, 0.0f));

				glBindVertexArray(data.vao);
				glUniformMatrix4fv(1, 1, GL_FALSE, &mvp[0][0]);
				glDrawElements(GL_TRIANGLES, data.elementCount, GL_UNSIGNED_SHORT, 0);
			}
		}
	}

	void MapRenderSystem::updateChunk(const MapChunk& chunk) {
		const auto index = chunkToIndex(chunk.getPosition());
		auto& data = chunkRenderData[index.x][index.y];
		if (data.chunk == &chunk) {
			data.updated = true;
		}
	}
	
	glm::ivec2 MapRenderSystem::chunkToIndex(const glm::ivec2 chunk) const {
		return (activeArea + chunk % activeArea) % activeArea;
	}

	void MapRenderSystem::updateChunkRenderData(RenderData& data) {
		data.updated = false;
		bool used[MapChunk::size.x][MapChunk::size.y] = {};
		// TODO: make `StaticVector`s?
		std::vector<Vertex> vboData;
		std::vector<GLushort> eboData;

		// TODO: Reserve vectors
		//vboData.reserve(elementCount); // NOTE: This is only an estimate. the correct ratio would be `c * 4/6.0f`
		//eboData.reserve(elementCount);

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

				for (bool cond = true; cond;) {
					std::fill(&used[end.x][begin.y], &used[end.x][end.y], true);
					++end.x;

					if (end.x == MapChunk::size.x) { break; }
					for (int y = begin.y; y < end.y; ++y) {
						if (!usable({end.x, y}, blockType)) { cond = false; break; }
					}
				}

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

		glDeleteBuffers(data.numBuffers, data.buffers);
		glCreateBuffers(data.numBuffers, data.buffers);

		glNamedBufferData(data.ebo, sizeof(eboData[0]) * eboData.size(), eboData.data(), GL_STATIC_DRAW);
		glVertexArrayElementBuffer(data.vao, data.ebo);

		glNamedBufferData(data.vbo, sizeof(vboData[0]) * vboData.size(), vboData.data(), GL_STATIC_DRAW);
		glVertexArrayVertexBuffer(data.vao, bufferBindingIndex, data.vbo, 0, sizeof(Vertex));
	}
}
