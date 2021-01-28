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
				ensurePlayAreaLoaded(ply);
			}
		}
	}

	void MapSystem::chunkFromNet(Connection& from, const Engine::Net::MessageHeader& head) {
		const byte* begin = reinterpret_cast<const byte*>(from.read(head.size));
		const byte* end = begin + head.size;
		glm::ivec2 chunkPos;

		memcpy(&chunkPos.x, begin, sizeof(chunkPos.x));
		begin += sizeof(chunkPos.x);
		
		memcpy(&chunkPos.y, begin, sizeof(chunkPos.y));
		begin += sizeof(chunkPos.y);

		ENGINE_LOG("Recv chunk ", head.seq, " (", chunkPos.x, ", ", chunkPos.y, ")");

		const auto regionPos = chunkToRegion(chunkPos);
		auto regionIt = regions.find(regionPos);
		const auto chunkIdx = chunkToRegionIndex(chunkPos);
		auto& chunk = regionIt->second->data[chunkIdx.x][chunkIdx.y];

		chunk.fromRLE(
			reinterpret_cast<const MapChunk::RLEPair*>(begin),
			reinterpret_cast<const MapChunk::RLEPair*>(end)
		);
		chunk.updated = world.getTick() + 1;
	}

	void MapSystem::run(float32 dt) {
		const auto tick = world.getTick();
		auto timeout = world.getTickTime() - std::chrono::seconds{10};

		for (auto ent : world.getFilter<MapAreaComponent>()) {
			auto& mapAreaComp = world.getComponent<MapAreaComponent>(ent);
			const auto begin = mapAreaComp.updates.begin();
			const auto end = mapAreaComp.updates.end();

			for (auto it = begin; it != end;) {
				const auto& chunkPos = it->first;
				auto& meta = it->second;

				if (meta.tick != tick) {
					ENGINE_LOG("Remove: ", chunkPos.x, " ", chunkPos.y);
					it = mapAreaComp.updates.erase(it);
					continue;
				}
				
				const auto regionPos = chunkToRegion(chunkPos);
				auto regionIt = regions.find(regionPos);
				const auto chunkIdx = chunkToRegionIndex(chunkPos);
				auto& chunk = regionIt->second->data[chunkIdx.x][chunkIdx.y];

				if (meta.last != chunk.updated) {
					chunk.toRLE();
					meta.last = chunk.updated;

					auto& connComp = world.getComponent<ConnectionComponent>(ent);
					auto& conn = *connComp.conn;
					if (auto msg = conn.beginMessage<MessageType::MAP_CHUNK>()) {
						const auto size = static_cast<int32>(chunk.encoding.size() * sizeof(chunk.encoding[0]));
						byte* data = reinterpret_cast<byte*>(chunk.encoding.data());
						ENGINE_LOG("Send Chunk: ", tick, " ", chunkPos.x, " ", chunkPos.y, " ", size);

						memcpy(data, &chunkPos.x, sizeof(chunkPos.x));
						memcpy(data + sizeof(chunkPos.x), &chunkPos.y, sizeof(chunkPos.y));
						msg.writeBlob(data, size);
						//static byte blob[1024 * 16] = {};
						//
						//memset(blob, 'z', sizeof(blob));
						//
						//for (int i = 0; i < sizeof(blob); ++i) {
						//	blob[i] = i;
						//}
						//
						//msg.writeBlob(blob, sizeof(blob));
					}
				}

				++it;
			}

			for (const auto pair : mapAreaComp.updates) {

				const auto& meta = pair.second;
				// TODO: erase, cant use range for loops
				if (meta.tick != tick) {

					ENGINE_LOG("MapArea: (", pair.first.x, ", ", pair.first.y, ") ", pair.second.tick, " ", tick);
				}

			}
		}

		// Unload regions
		for (auto it = regions.begin(); it != regions.end();) {
			if (it->second->lastUsed < timeout) {
				ENGINE_LOG("Unloading region: ", it->first.x, ", ", it->first.y);
				it = regions.erase(it);
			} else {
				++it;
			}
		}

		// Unload active chunks
		for (auto it = activeChunks.begin(); it != activeChunks.end();) {
			if (it->second.lastUsed < timeout) {
				ENGINE_LOG("Unloading chunk: ", it->first.x, ", ", it->first.y);
				world.getSystem<PhysicsSystem>().destroyBody(it->second.body);
				it = activeChunks.erase(it);
			} else {
				++it;
			}
		}
	}

	void MapSystem::ensurePlayAreaLoaded(Engine::ECS::Entity ply) {
		#if ENGINE_SERVER
			auto& mapAreaComp = world.getComponent<MapAreaComponent>(ply);
		#endif
		const auto tick = world.getTick();
		const auto plyPos = Engine::Glue::as<glm::vec2>(world.getComponent<PhysicsBodyComponent>(ply).getPosition());
		const auto blockPos = worldToBlock(plyPos);

		// How large of an area to load around the chunk blockPos is in.
		constexpr auto areaSize = glm::ivec2{2, 2};

		// How large of a buffer around areaSize to wait befor unloading areas.
		// We want a larger unload area so that we arent constantly loading/unloading
		// when a player is near a chunk border
		constexpr auto buffSize = glm::ivec2{2, 2}; // TODO: i think we may want this mouse around 3-5 range

		const auto minAreaChunk = blockToChunk(blockPos) - areaSize;
		const auto maxAreaChunk = blockToChunk(blockPos) + areaSize;
		const auto minBuffChunk = minAreaChunk - buffSize;
		const auto maxBuffChunk = maxAreaChunk + buffSize;

		for (auto chunkPos = minBuffChunk; chunkPos.x <= maxBuffChunk.x; ++chunkPos.x) {
			for (chunkPos.y = minBuffChunk.y; chunkPos.y <= maxBuffChunk.y; ++chunkPos.y) {
				const auto isBufferChunk = chunkPos.x < minAreaChunk.x
					|| chunkPos.x > maxAreaChunk.x
					|| chunkPos.y < minAreaChunk.y
					|| chunkPos.y > maxAreaChunk.y;

				const auto regionPos = chunkToRegion(chunkPos);
				auto regionIt = regions.find(regionPos);

				if (regionIt == regions.end()) {
					if (!isBufferChunk) {
						const auto it = regions.emplace(regionPos, new MapRegion{
							.lastUsed = world.getTickTime(),
						}).first;

						if constexpr (ENGINE_SERVER) {
							queueRegionToLoad(regionPos, *it->second);
						}
					}
					continue;
				}

				const auto& region = regionIt->second;
				if (region->loading()) { continue; }
				region->lastUsed = world.getTickTime();

				#if ENGINE_SERVER
				{
					const auto found = mapAreaComp.updates.contains(chunkPos);
					if (!isBufferChunk || found) {
						mapAreaComp.updates[chunkPos].tick = world.getTick();
					}
				}
				#endif

				const auto chunkIdx = chunkToRegionIndex(chunkPos);
				auto& chunk = region->data[chunkIdx.x][chunkIdx.y];

				auto it = activeChunks.find(chunkPos);
				if (it == activeChunks.end()) {
					if (isBufferChunk) { continue; }
					it = activeChunks.emplace(chunkPos, TestData{}).first;
					it->second.body = createBody();
					setupMesh(it->second.mesh);
					ENGINE_LOG("Activating chunk: ", chunkPos.x, ", ", chunkPos.y, " (", (chunk.updated == tick) ? "fresh" : "stale", ")");
					chunk.updated = tick;
				}

				if (chunk.updated == tick) {
					buildActiveChunkData(it->second, chunk, chunkPos);
				}

				it->second.lastUsed = world.getTickTime();
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

		const auto regionPos = chunkToRegion(chunkPos);
		const auto regionIt = regions.find(regionPos);
		if (regionIt == regions.end() || regionIt->second->loading()) [[unlikely]] { return; }

		auto& chunk = regionIt->second->data[chunkIndex.x][chunkIndex.y];

		if (chunk.data[blockIndex.x][blockIndex.y] != value) {
			chunk.data[blockIndex.x][blockIndex.y] = value;
			chunk.updated = world.getTick();
		}
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
				//chunk.data[bpos.x][bpos.y] = (bpos.x == 0) ^ (bpos.y == 0);
				//chunk.data[bpos.x][bpos.y] = bpos.x & 1 || bpos.y & 1;
			}
		}

		chunk.updated = world.getTick();
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
