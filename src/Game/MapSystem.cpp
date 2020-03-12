// GLM
#include <glm/gtc/matrix_transform.hpp>

// Engine
#include <Engine/Glue/Box2D.hpp>

// Game
#include <Game/MapSystem.hpp>
#include <Game/PhysicsSystem.hpp>
#include <Game/SpriteSystem.hpp>
#include <Game/CameraTrackingSystem.hpp>
#include <Game/World.hpp>


namespace Game {
	MapSystem::MapSystem(SystemArg arg)
		: System{arg} {
		static_assert(World::orderAfter<MapSystem, CameraTrackingSystem>());
	}

	MapSystem::~MapSystem() {
	}

	void MapSystem::setup() {
		shader = engine.shaderManager.get("shaders/terrain");
		texture = engine.textureManager.get("../assets/test.png");
		mapEntity = world.createEntity();
		auto& physSys = world.getSystem<PhysicsSystem>();

		// TODO: Once you change ECS::SystemManager to store instances on the class itself instead of with `new`, this will no longer be needed.

		std::fill(&loadedRegions[0][0], &loadedRegions[0][0] + regionCount.x * regionCount.y, glm::ivec2{0x7FFF'FFFF, 0x7FFF'FFFF});

		// Active Area stuff
		b2BodyDef bodyDef;
		bodyDef.type = b2_staticBody;
		bodyDef.awake = false;
		bodyDef.fixedRotation = true;

		constexpr Engine::Graphics::VertexFormat<1> vertexFormat = {
			sizeof(Vertex),
			{{.location = 0, .size = 2, .type = GL_FLOAT, .offset = offsetof(Vertex, pos)}}
		};

		for (int x = 0; x < activeAreaSize.x; ++x) {
			for (int y = 0; y < activeAreaSize.y; ++y) {
				auto& data = activeAreaData[x][y];
				data.body = physSys.createBody(mapEntity, bodyDef);
				data.chunkPos = glm::ivec2{0x7FFF'FFFF, 0x7FFF'FFFF};
				data.mesh.setBufferFormat(vertexFormat);
			}
		}
	}

	void MapSystem::run(float dt) {
		updateOrigin();
		const auto minChunk = blockToChunk(worldToBlock(engine.camera.getWorldScreenBounds().min)) - glm::ivec2{1, 1};
		const auto maxChunk = blockToChunk(worldToBlock(engine.camera.getWorldScreenBounds().max)) + glm::ivec2{1, 1};
		
		// TODO: Handle chunk/region loading in different thread	
		// As long as screen size < region size we only need to check the four corners
		const auto minRegion = chunkToRegion(minChunk);
		const auto maxRegion = chunkToRegion(maxChunk);
		ensureRegionLoaded(minRegion);
		ensureRegionLoaded(maxRegion);
		ensureRegionLoaded({minRegion.x, maxRegion.y});
		ensureRegionLoaded({maxRegion.x, minRegion.y});

		for (auto chunk = minChunk; chunk.x <= maxChunk.x; ++chunk.x) {
			for (chunk.y = minChunk.y; chunk.y <= maxChunk.y; ++chunk.y) {
				const auto idx = chunkToActiveIndex(chunk);
				auto& data = activeAreaData[idx.x][idx.y];

				if (data.updated || data.chunkPos != chunk) {
					data.chunkPos = chunk;
					buildActiveChunkData(data);
				}
			}
		}
	}
	
	const glm::ivec2& MapSystem::getChunkOffset() const {
		return mapOffset;
	}

	glm::ivec2 MapSystem::getBlockOffset() const {
		return mapOffset * MapChunk::size;
	}

	void MapSystem::setValueAt(const glm::vec2 wpos, int value) {
		// TODO: Make conversion functions for all of these? Better names

		const auto blockOffset = glm::floor(wpos / MapChunk::blockSize);
		const auto blockIndex = (MapChunk::size + glm::ivec2{blockOffset} % MapChunk::size) % MapChunk::size;

		//std::cout << "Bidx: " << blockIndex.x << ", " << blockIndex.y << "\n";
		const glm::ivec2 chunkOffset = glm::floor(blockOffset / glm::vec2{MapChunk::size});
		const auto chunkPos = mapOffset + chunkOffset;
		const auto chunkIndex = (mapSize + glm::ivec2{chunkPos} % mapSize) % mapSize;

		auto& chunk = chunks[chunkIndex.x][chunkIndex.y];
		chunk.data[blockIndex.x][blockIndex.y] = value;

		// TODO: we really only want to generate once if we the chunk has been changed since last frame.
		// TODO: as it is now we generate once per edit, which could be many times per frame

		const auto aidx = chunkToActiveIndex(chunkPos);
		auto& adata = activeAreaData[aidx.x][aidx.y];
		if (adata.chunkPos == chunkPos) {
			adata.updated = true;
		}
	}
	
	glm::ivec2 MapSystem::worldToBlock(const glm::vec2 world) const {
		const glm::ivec2 blockOffset = glm::floor(world / MapChunk::blockSize);
		return getBlockOffset() + blockOffset;
	}

	glm::vec2 MapSystem::blockToWorld(const glm::ivec2 block) const {
		return glm::vec2{block - mapOffset * MapChunk::size} * MapChunk::blockSize;
	}

	glm::ivec2 MapSystem::blockToChunk(const glm::ivec2 block) const {
		// Integer division + floor
		auto d = block / MapChunk::size;
		d.x = d.x * MapChunk::size.x == block.x ? d.x : d.x - (block.x < 0);
		d.y = d.y * MapChunk::size.y == block.y ? d.y : d.y - (block.y < 0);
		return d;
	}

	glm::ivec2 MapSystem::chunkToBlock(const glm::ivec2 chunk) const {
		return chunk * MapChunk::size;
	}

	glm::ivec2 MapSystem::chunkToRegion(const glm::ivec2 chunk) const {
		// Integer division + floor
		auto d = chunk / regionSize;
		d.x = d.x * regionSize.x == chunk.x ? d.x : d.x - (chunk.x < 0);
		d.y = d.y * regionSize.y == chunk.y ? d.y : d.y - (chunk.y < 0);
		return d;
	}

	glm::ivec2 MapSystem::chunkToIndex(const glm::ivec2 chunk) const {
		return (mapSize + chunk % mapSize) % mapSize;
	}

	glm::ivec2 MapSystem::chunkToActiveIndex(const glm::ivec2 chunk) const {
		return (activeAreaSize + chunk % activeAreaSize) % activeAreaSize;
	}

	glm::ivec2 MapSystem::regionToChunk(const glm::ivec2 region) const {
		return region * regionSize;
	}

	glm::ivec2 MapSystem::regionToIndex(const glm::ivec2 region) const {
		return (regionCount + region % regionCount) % regionCount;
	}

	MapChunk& MapSystem::getChunkAt(glm::ivec2 chunk) {
		const auto pos = chunkToIndex(chunk);
		return chunks[pos.x][pos.y];
	}

	const MapChunk& MapSystem::getChunkAt(glm::ivec2 chunk) const {
		return const_cast<MapSystem*>(this)->getChunkAt(chunk);
	}

	void MapSystem::buildActiveChunkData(ActiveChunkData& data) {
		data.updated = false;
		// TODO: simplify. currently have two mostly duplicate sections.
		const auto idx = chunkToIndex(data.chunkPos);
		const auto& chunk = chunks[idx.x][idx.y];

		{ // Render stuff
			bool used[MapChunk::size.x][MapChunk::size.y] = {};

			const auto usable = [&](const glm::ivec2 pos, const int blockType) {
				return !used[pos.x][pos.y] && chunk.data[pos.x][pos.y] == blockType;
			};

			for (glm::ivec2 begin = {0, 0}; begin.x < MapChunk::size.x; ++begin.x) {  
				for (begin.y = 0; begin.y < MapChunk::size.y; ++begin.y) {
					// Greedy expand
					const auto blockType = chunk.data[begin.x][begin.y];
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
					glm::vec2 origin = glm::vec2{begin} * MapChunk::blockSize;
					glm::vec2 size = glm::vec2{end - begin} * MapChunk::blockSize;
					const auto vertexCount = static_cast<GLushort>(buildVBOData.size());

					buildVBOData.push_back({origin});
					buildVBOData.push_back({origin + glm::vec2{size.x, 0}});
					buildVBOData.push_back({origin + size});
					buildVBOData.push_back({origin + glm::vec2{0, size.y}});

					buildEBOData.push_back(vertexCount + 0);
					buildEBOData.push_back(vertexCount + 1);
					buildEBOData.push_back(vertexCount + 2);
					buildEBOData.push_back(vertexCount + 2);
					buildEBOData.push_back(vertexCount + 3);
					buildEBOData.push_back(vertexCount + 0);
				}
			}

			data.mesh.setBufferData(buildVBOData, buildEBOData);

			buildVBOData.clear();
			buildEBOData.clear();
		}

		{ // Physics stuff
			const auto pos = Engine::Glue::as<b2Vec2>(blockToWorld(chunkToBlock(data.chunkPos)));
			data.body->SetTransform(pos, 0);
			// TODO: Look into edge and chain shapes
			// Clear all fixtures
			for (auto* fixture = data.body->GetFixtureList(); fixture;) {
				auto* next = fixture->GetNext();
				data.body->DestroyFixture(fixture);
				fixture = next;
			}

			b2PolygonShape shape;
			b2FixtureDef fixtureDef;
			fixtureDef.shape = &shape;

			bool used[MapChunk::size.x][MapChunk::size.y]{};

			// TODO: For collision we only want to check if a block is solid or not. We dont care about type.
			const auto usable = [&](const glm::ivec2 pos, const int blockType) {
				return !used[pos.x][pos.y] && chunk.data[pos.x][pos.y] == blockType;
			};
			
			for (glm::ivec2 begin = {0, 0}; begin.x < MapChunk::size.x; ++begin.x) {
				for (begin.y = 0; begin.y < MapChunk::size.y; ++begin.y) {
					// Greedy expand
					const auto blockType = chunk.data[begin.x][begin.y];
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

					// Add physics data
					const auto halfSize = MapChunk::blockSize * 0.5f * Engine::Glue::as<b2Vec2>(end - begin);
					const auto center = MapChunk::blockSize * Engine::Glue::as<b2Vec2>(begin) + halfSize;

					shape.SetAsBox(halfSize.x, halfSize.y, center, 0.0f);
					data.body->CreateFixture(&fixtureDef);
				}
			}
		}
	}

	void MapSystem::ensureRegionLoaded(const glm::ivec2 region) {
		// TODO: Consider combining this and loadRegion. We never call one without the other and they could share some data.
		const auto index = regionToIndex(region);

		if (loadedRegions[index.x][index.y] != region) {
			loadRegion(region);
		}
	}

	void MapSystem::loadRegion(const glm::ivec2 region) {
		std::cout << "loadRegion: " << "(" << region.x << ", " << region.y << ")\n";

		const auto regionStart = regionToChunk(region);

		const auto index = regionToIndex(region);
		loadedRegions[index.x][index.y] = region;

		for (int x = 0; x < regionSize.x; ++x) {
			for (int y = 0; y < regionSize.y; ++y) {
				const auto chunk = regionStart + glm::ivec2{x, y};
				loadChunk(chunk);
			}
		}
	}

	void MapSystem::loadChunk(const glm::ivec2 pos) {
		auto& chunk = getChunkAt(pos);
		const auto chunkBlockPos = chunkToBlock(pos);

		for (glm::ivec2 bpos = {0, 0}; bpos.x < MapChunk::size.x; ++bpos.x) {
			for (bpos.y = 0; bpos.y < MapChunk::size.y; ++bpos.y) {
				const auto absPos = chunkBlockPos + bpos;
				int block = 0;
		
				if (0 < mgen.value(absPos.x, absPos.y)) {
					block = 1;
				}
		
				chunk.data[bpos.x][bpos.y] = block;
				//chunk.data[bpos.x][bpos.y] = bpos.x == 0 || bpos.y == 0;
				//chunk.data[bpos.x][bpos.y] = bpos.x & 1 || bpos.y & 1;
			}
		}

		chunk.pos = pos;
	}

	void MapSystem::updateOrigin() {
		// TODO: Move to own system. This doesnt really depend on the map.
		const auto& pos = engine.camera.getPosition();
		constexpr auto range = glm::vec2{MapChunk::size * originRange} * MapChunk::blockSize;

		if (std::abs(pos.x) > range.x) {
			auto& physSys = world.getSystem<Game::PhysicsSystem>();
			auto dir = std::copysign(1.0f, pos.x);

			physSys.getWorld().ShiftOrigin(b2Vec2{
				range.x * dir,
				0.0f
			});

			mapOffset.x += static_cast<int>(dir) * originRange;

			// TODO: Figure out a better way to update camera. This seems hacky.
			// TODO: Doing this could also cause problems with other things that use the camera.
			world.getSystem<CameraTrackingSystem>().run(0.0f);
		}

		if (std::abs(pos.y) > range.y) {
			auto& physSys = world.getSystem<Game::PhysicsSystem>();
			auto dir = std::copysign(1.0f, pos.y);

			physSys.getWorld().ShiftOrigin(b2Vec2{
				0.0f,
				range.y * dir
			});

			mapOffset.y += static_cast<int>(dir) * originRange;

			// TODO: Figure out a better way to update camera. This seems hacky.
			// TODO: Doing this could also cause problems with other things that use the camera.
			world.getSystem<CameraTrackingSystem>().run(0.0f);
		}
	}
}
