// GLM
#include <glm/gtc/matrix_transform.hpp>

// Engine
#include <Engine/Glue/Box2D.hpp>
#include <Engine/Glue/glm.hpp>

// Game
#include <Game/systems/MapSystem.hpp>
#include <Game/systems/PhysicsSystem.hpp>
#include <Game/systems/SpriteSystem.hpp>
#include <Game/systems/CameraTrackingSystem.hpp>
#include <Game/World.hpp>
#include <Game/systems/PhysicsOriginShiftSystem.hpp>


namespace {
	using PlayerFilter = Engine::ECS::EntityFilterList<
		Game::PlayerFlag,
		Game::PhysicsBodyComponent,
		Game::ConnectionComponent
	>;
}


namespace Game {
	MapSystem::MapSystem(SystemArg arg)
		: System{arg} {
		static_assert(World::orderAfter<MapSystem, CameraTrackingSystem>());

		for (auto& t : threads) {
			t = std::thread{&MapSystem::loadChunkAsyncWorker, this};
		}
	}

	MapSystem::~MapSystem() {
		threadsShouldExit = true;
		condv.notify_all();

		for (auto& t : threads) {
			t.join();
		}
	}

	void MapSystem::setup() {
		mapEntity = world.createEntity();
		shader = engine.shaderManager.get("shaders/terrain");
		texture = engine.textureManager.get("assets/test.png");
	}

	b2Body* MapSystem::createBody() {
		b2BodyDef bodyDef;
		bodyDef.type = b2_staticBody;
		bodyDef.awake = false;
		bodyDef.fixedRotation = true;
		return world.getSystem<PhysicsSystem>().createBody(mapEntity, bodyDef);
	}

	void MapSystem::setupMesh(Engine::Graphics::Mesh& mesh) const {
		constexpr Engine::Graphics::VertexFormat<1> vertexFormat = {
			sizeof(Vertex),
			{{.location = 0, .size = 2, .type = GL_FLOAT, .offset = offsetof(Vertex, pos)}}
		};

		mesh.setBufferFormat(vertexFormat);
	}

	void MapSystem::tick() {
		// TODO: move
		const auto makeEdit = [&](int value, const glm::vec2 mouse) {
			for (int x = -1; x < 2; ++x) {
				for (int y = -1; y < 2; ++y) {
					setValueAt(
						mouse + glm::vec2{x * MapChunk::blockSize, y * MapChunk::blockSize},
						value
					);
				}
			}
		};

		for (auto& ply : world.getFilter<PlayerFilter>()) {
			const auto& actComp = world.getComponent<ActionComponent>(ply);

			if (actComp.getButton(Button::Attack1).latest) {
				const auto& physBodyComp = world.getComponent<PhysicsBodyComponent>(ply);
				const auto& pos = Engine::Glue::as<glm::vec2>(physBodyComp.getPosition());
				makeEdit(1, pos + actComp.getTarget());
			}
			if (actComp.getButton(Button::Attack2).latest) {
				const auto& physBodyComp = world.getComponent<PhysicsBodyComponent>(ply);
				const auto& pos = Engine::Glue::as<glm::vec2>(physBodyComp.getPosition());
				makeEdit(0, pos + actComp.getTarget());
			}

			if (!world.isPerformingRollback()) {
				auto pos = Engine::Glue::as<glm::vec2>(world.getComponent<PhysicsBodyComponent>(ply).getPosition());
				ensurePlayAreaLoaded(worldToBlock(pos));
			}
		}
	}

	void MapSystem::run(float32 dt) {
		auto timeout = world.getTickTime() - std::chrono::seconds{10};
		for (auto it = regions.begin(); it != regions.end();) {
			if (it->second->lastUsed < timeout) {
				std::cout << "Unloading region: " << it->first.x << ", " << it->first.y << "\n";
				it = regions.erase(it);
			} else {
				++it;
			}
		}
	}

	void MapSystem::ensurePlayAreaLoaded(glm::ivec2 blockPos) {
		const auto minChunk = blockToChunk(blockPos) - glm::ivec2{2,2};
		const auto maxChunk = blockToChunk(blockPos) + glm::ivec2{2,2};

		for (auto chunkPos = minChunk; chunkPos.x <= maxChunk.x; ++chunkPos.x) {
			for (chunkPos.y = minChunk.y; chunkPos.y <= maxChunk.y; ++chunkPos.y) {
				auto& region = ensureRegionLoaded(chunkToRegion(chunkPos));
				if (region.loading()) { continue; }
				region.lastUsed = world.getTickTime();

				const auto idx = chunkToRegionIndex(chunkPos);
				auto& chunk = region.data[idx.x][idx.y];

				// TODO: at no point are active chunks cleaned up atm
				auto it = activeChunks.find(chunkPos);
				if (it == activeChunks.end()) {
					it = activeChunks.emplace(chunkPos, TestData{}).first;

					// TODO: these need to be freed when destructing
					it->second.body = createBody();
					setupMesh(it->second.mesh);
				}

				if (chunk.updated) {
					chunk.updated = false;
					buildActiveChunkData(it->second, chunk, chunkPos);
				}

				it->second.lastTouched = world.getTick();
			}
		}
	}
	
	glm::ivec2 MapSystem::getBlockOffset() const {
		constexpr int32 blocksPerShift = static_cast<int32>(PhysicsOriginShiftSystem::range / MapChunk::blockSize);
		static_assert(PhysicsOriginShiftSystem::range - blocksPerShift * MapChunk::blockSize == 0.0f, "Remainder not handled");
		return blocksPerShift * world.getSystem<PhysicsOriginShiftSystem>().getOffset();
	}

	void MapSystem::setValueAt(const glm::vec2 wpos, int value) {
		// TODO: Make conversion functions for all of these? Better names

		const auto blockOffset = glm::floor(wpos / MapChunk::blockSize);
		const auto blockIndex = (MapChunk::size + glm::ivec2{blockOffset} % MapChunk::size) % MapChunk::size;

		const glm::ivec2 chunkOffset = glm::floor(blockOffset / glm::vec2{MapChunk::size});
		const auto chunkPos = blockToChunk(getBlockOffset()) + chunkOffset;
		const auto chunkIndex = chunkToRegionIndex(chunkPos);
		auto& region = ensureRegionLoaded(chunkToRegion(chunkPos));

		// TODO: should we wait/spin till load?
		if (region.loading()) [[unlikely]] { return; }

		auto& chunk = region.data[chunkIndex.x][chunkIndex.y];
		chunk.data[blockIndex.x][blockIndex.y] = value;
		chunk.updated = true;

		// TODO: we really only want to generate once if we the chunk has been changed since last frame.
		// TODO: as it is now we generate once per edit, which could be many times per frame
	}
	
	glm::ivec2 MapSystem::worldToBlock(const glm::vec2 world) const {
		const glm::ivec2 blockOffset = glm::floor(world / MapChunk::blockSize);
		return getBlockOffset() + blockOffset;
	}

	glm::vec2 MapSystem::blockToWorld(const glm::ivec2 block) const {
		return glm::vec2{block - getBlockOffset()} * MapChunk::blockSize;
	}

	// TODO: thread this. Not sure how nice box2d will play with it.
	void MapSystem::buildActiveChunkData(TestData& data, const MapChunk& chunk, glm::ivec2 chunkPos) {
		// TODO: simplify. currently have two mostly duplicate sections.
		
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
			const auto pos = Engine::Glue::as<b2Vec2>(blockToWorld(chunkToBlock(chunkPos)));
			auto& body = *data.body;

			// TODO: Look into edge and chain shapes
			// Clear all fixtures
			for (auto* fixture = body.GetFixtureList(); fixture;) {
				auto* next = fixture->GetNext();
				body.DestroyFixture(fixture);
				fixture = next;
			}

			body.SetTransform(pos, 0);

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
					body.CreateFixture(&fixtureDef);
				}
			}
		}
	}

	MapRegion& MapSystem::ensureRegionLoaded(const glm::ivec2 regionPos) {
		auto it = regions.find(regionPos);
		if (it == regions.end()) {
			it = regions.emplace(regionPos, new MapRegion{
				.lastUsed = world.getTickTime(),
			}).first;
			queueRegionToLoad(regionPos, *it->second);
		}
		return *it->second;
	}

	void MapSystem::loadChunk(const glm::ivec2 chunkPos, MapChunk& chunk) {
		const auto chunkBlockPos = chunkToBlock(chunkPos);

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

		chunk.updated = true;
	}

	void MapSystem::loadChunkAsyncWorker() {
		while(true) {
			std::unique_lock lock{chunksToLoadMutex};

			while (chunksToLoad.empty()) {
				condv.wait(lock);
				if (threadsShouldExit) { return; }
			}

			const auto job = chunksToLoad.front();
			chunksToLoad.pop();
			lock.unlock();

			loadChunk(job.chunkPos, job.chunk);
			++job.region.loadedChunks;
		}
	}

	void MapSystem::queueRegionToLoad(glm::ivec2 regionPos, MapRegion& region) {
		std::cout << "Queue region: " << regionPos.x << " " << regionPos.y << "\n";
		const auto regionStart = regionToChunk(regionPos);

		std::unique_lock lock{chunksToLoadMutex};

		for (int x = 0; x < regionSize.x; ++x) {
			for (int y = 0; y < regionSize.y; ++y) {
				const auto chunkPos = regionStart + glm::ivec2{x, y};
				auto& chunk = region.data[x][y];
				chunksToLoad.push({chunkPos, region, chunk});
			}
		}

		lock.unlock();
		condv.notify_all();
	}
}
