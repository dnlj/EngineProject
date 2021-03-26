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

		constexpr int32 offset = 2; // offset by 2 to skip None and Air
		const char* textures[BlockId::_COUNT - offset] = {};
		for (int32 i = offset; i < BlockId::_COUNT; ++i) {
			textures[i - offset] = getBlockMeta(static_cast<BlockId>(i)).path;
		}

		// TODO: really no reason to use RGBA here. we dont use alpha
		texArr.setStorage(Engine::TextureFormat::SRGBA8, {32, 32, std::size(textures)});

		Engine::Image img;
		for (int i = 0; auto path : textures) {
			img = path;
			img.flipY();
			texArr.setSubImage(0, {0, 0, i++}, {img.size(), 1}, img);
		}
	}

	b2Body* MapSystem::createBody() {
		b2BodyDef bodyDef;
		bodyDef.type = b2_staticBody;
		bodyDef.awake = false;
		bodyDef.fixedRotation = true;
		return world.getSystem<PhysicsSystem>().createBody(mapEntity, bodyDef);
	}

	void MapSystem::setupMesh(Engine::Graphics::Mesh& mesh) const {
		constexpr Engine::Graphics::VertexFormat<2> vertexFormat = {
			sizeof(Vertex),
			{
				{.location = 0, .size = 2, .type = GL_FLOAT, .offset = offsetof(Vertex, pos)},
				{.location = 1, .size = 1, .type = GL_FLOAT, .offset = offsetof(Vertex, tex)},
			}
		};

		mesh.setBufferFormat(vertexFormat);
	}

	void MapSystem::tick() {
		const auto currTick = world.getTick();

		// TODO: move
		const auto makeEdit = [&](BlockId bid, const glm::vec2 mouse) {
			for (int x = -1; x < 2; ++x) {
				for (int y = -1; y < 2; ++y) {
					setValueAt(
						mouse + glm::vec2{x * MapChunk::blockSize, y * MapChunk::blockSize},
						bid
					);
				}
			}
		};

		for (auto& ply : world.getFilter<PlayerFilter>()) {
			const auto& actComp = world.getComponent<ActionComponent>(ply);

			if (actComp.getButton(Button::Attack1).latest) {
				const auto& physBodyComp = world.getComponent<PhysicsBodyComponent>(ply);
				const auto& pos = Engine::Glue::as<glm::vec2>(physBodyComp.getPosition());
				makeEdit(BlockId::Dirt, pos + actComp.getTarget());
			}
			if (actComp.getButton(Button::Attack2).latest) {
				const auto& physBodyComp = world.getComponent<PhysicsBodyComponent>(ply);
				const auto& pos = Engine::Glue::as<glm::vec2>(physBodyComp.getPosition());
				makeEdit(BlockId::Air, pos + actComp.getTarget());
			}

			if (!world.isPerformingRollback()) {
				ensurePlayAreaLoaded(ply);
			}
		}

		// Apply edits
		for (auto& [chunkPos, edit] : chunkEdits) {
			const auto regionPos = chunkToRegion(chunkPos);
			const auto regionIt = regions.find(regionPos);
			if (regionIt == regions.end() || regionIt->second->loading()) [[unlikely]] {
				ENGINE_WARN("Attempting to edit unloaded chunk/region");
				continue;
			}

			const auto chunkIndex = chunkToRegionIndex(chunkPos);
			auto& chunk = regionIt->second->data[chunkIndex.x][chunkIndex.y];

			if (chunk.apply(edit)) {
				const auto found = activeChunks.find(chunkPos);
				if (found != activeChunks.end()) {
					found->second.updated = currTick;
				}
			}
		}

		for (auto& [chunkPos, activeData] : activeChunks) {
			if (activeData.updated == currTick) {
				buildActiveChunkData(activeData, chunkPos);
			}
		}

		chunkEdits.clear();
	}

	void MapSystem::chunkFromNet(Connection& from, const Engine::Net::MessageHeader& head) {
		const byte* begin = reinterpret_cast<const byte*>(from.read(head.size));
		const byte* end = begin + head.size;
		glm::ivec2 chunkPos;

		memcpy(&chunkPos.x, begin, sizeof(chunkPos.x));
		begin += sizeof(chunkPos.x);
		
		memcpy(&chunkPos.y, begin, sizeof(chunkPos.y));
		begin += sizeof(chunkPos.y);

		chunkEdits[chunkPos].fromRLE(begin, end);
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
				
				const auto found = activeChunks.find(chunkPos);
				if (found == activeChunks.end()) { continue; }
				auto& activeData = found->second;

				if (meta.last != activeData.updated) {
					if (meta.last == 0) { // Fresh chunk
						auto& connComp = world.getComponent<ConnectionComponent>(ent);
						auto& conn = *connComp.conn;

						const auto regionPos = chunkToRegion(chunkPos);
						const auto regionIt = regions.find(regionPos);
						if (regionIt != regions.end() && !regionIt->second->loading()) {
							const auto chunkIndex = chunkToRegionIndex(chunkPos);
							auto& chunk = regionIt->second->data[chunkIndex.x][chunkIndex.y];
							rleTemp.clear();
							chunk.toRLE(rleTemp);
							
							if (auto msg = conn.beginMessage<MessageType::MAP_CHUNK>()) {
								meta.last = activeData.updated;
								const auto size = static_cast<int32>(rleTemp.size() * sizeof(rleTemp[0]));
								byte* data = reinterpret_cast<byte*>(rleTemp.data());
								// ENGINE_LOG("Send Chunk (fresh): ", tick, " ", chunkPos.x, " ", chunkPos.y, " ", size);
								memcpy(data, &chunkPos.x, sizeof(chunkPos.x));
								memcpy(data + sizeof(chunkPos.x), &chunkPos.y, sizeof(chunkPos.y));
								msg.writeBlob(data, size);
							}
						}
					} else if (activeData.rle.empty()) {
						// TODO: i dont think this case should be hit?
						ENGINE_WARN("No RLE data for chunk");
					} else { // Chunk edit
						auto& connComp = world.getComponent<ConnectionComponent>(ent);
						auto& conn = *connComp.conn;
						if (auto msg = conn.beginMessage<MessageType::MAP_CHUNK>()) {
							meta.last = activeData.updated;
							const auto size = static_cast<int32>(activeData.rle.size() * sizeof(activeData.rle[0]));
							byte* data = reinterpret_cast<byte*>(activeData.rle.data());
							// ENGINE_LOG("Send Chunk (edit): ", tick, " ", chunkPos.x, " ", chunkPos.y, " ", size);
							memcpy(data, &chunkPos.x, sizeof(chunkPos.x));
							memcpy(data + sizeof(chunkPos.x), &chunkPos.y, sizeof(chunkPos.y));
							msg.writeBlob(data, size);
						}
					}
				}

				++it;
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
						mapAreaComp.updates[chunkPos].tick = tick;
					}
				}
				#endif
				
				auto it = activeChunks.find(chunkPos);
				if (it == activeChunks.end()) {
					if (isBufferChunk) { continue; }
					it = activeChunks.emplace(chunkPos, TestData{}).first;
					it->second.body = createBody();
					setupMesh(it->second.mesh);
					ENGINE_LOG("Activating chunk: ", chunkPos.x, ", ", chunkPos.y, " (", (it->second.updated == tick) ? "fresh" : "stale", ")");
					it->second.updated = tick;
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

	void MapSystem::setValueAt(const glm::vec2 wpos, BlockId bid) {
		// TODO: Make conversion functions for all of these? Better names

		const auto blockOffset = glm::floor(wpos / MapChunk::blockSize);
		const auto blockIndex = (MapChunk::size + glm::ivec2{blockOffset} % MapChunk::size) % MapChunk::size;

		const glm::ivec2 chunkOffset = glm::floor(blockOffset / glm::vec2{MapChunk::size});
		const auto chunkPos = blockToChunk(getBlockOffset()) + chunkOffset;

		auto& edit = chunkEdits[chunkPos];
		edit.data[blockIndex.x][blockIndex.y] = bid;
	}
	
	glm::ivec2 MapSystem::worldToBlock(const glm::vec2 world) const {
		const glm::ivec2 blockOffset = glm::floor(world / MapChunk::blockSize);
		return getBlockOffset() + blockOffset;
	}

	glm::vec2 MapSystem::blockToWorld(const glm::ivec2 block) const {
		return glm::vec2{block - getBlockOffset()} * MapChunk::blockSize;
	}

	// TODO: thread this. Not sure how nice box2d will play with it.
	void MapSystem::buildActiveChunkData(TestData& data, glm::ivec2 chunkPos) {
		const auto regionPos = chunkToRegion(chunkPos);
		const auto regionIt = regions.find(regionPos);
		if (regionIt == regions.end() || regionIt->second->loading()) [[unlikely]] { return; }
				
		const auto chunkIndex = chunkToRegionIndex(chunkPos);
		auto& chunk = regionIt->second->data[chunkIndex.x][chunkIndex.y];

		if constexpr (ENGINE_SERVER) { // Build edits
			const auto found = chunkEdits.find(chunkPos);
			if (found == chunkEdits.end()) {
				data.rle.clear();

				// TODO: i dont think this should ever happen because the only time we call this function is when we have an update?
				ENGINE_WARN("No chunk edits.");
			} else {
				found->second.toRLE(data.rle);
			}
		}

		decltype(auto) greedyExpand = [&chunk](auto usable, auto submitArea) ENGINE_INLINE {
			bool used[MapChunk::size.x][MapChunk::size.y] = {};

			for (glm::ivec2 begin = {0, 0}; begin.x < MapChunk::size.x; ++begin.x) {  
				for (begin.y = 0; begin.y < MapChunk::size.y; ++begin.y) {
					const auto& blockMeta = getBlockMeta(chunk.data[begin.x][begin.y]);
					// TODO: rm - old - if (blockType == MapChunk::AIR.id || !usable(begin, blockType)) { continue; }
					if (used[begin.x][begin.y] || !usable(begin, blockMeta)) { continue; }
					auto end = begin;

					while (end.y < MapChunk::size.y && !used[begin.x][begin.y] && usable(end, blockMeta)) { ++end.y; }

					for (bool cond = true; cond;) {
						std::fill(&used[end.x][begin.y], &used[end.x][end.y], true);
						++end.x;

						if (end.x == MapChunk::size.x) { break; }
						for (int y = begin.y; y < end.y; ++y) {
							if (used[begin.x][begin.y] || !usable(glm::ivec2{end.x, y}, blockMeta)) { cond = false; break; }
						}
					}

					submitArea(begin, end);
				}
			}
		};

		{ // Render
			greedyExpand([&](const auto& pos, const auto& blockMeta) ENGINE_INLINE {
				return blockMeta.id != BlockId::None
					&& blockMeta.id != BlockId::Air
					&& chunk.data[pos.x][pos.y] == blockMeta.id;
			}, [&](const auto& begin, const auto& end) ENGINE_INLINE {
				// Add buffer data
				glm::vec2 origin = glm::vec2{begin} * MapChunk::blockSize;
				glm::vec2 size = glm::vec2{end - begin} * MapChunk::blockSize;
				const auto vertexCount = static_cast<GLushort>(buildVBOData.size());

				static_assert(BlockId::_COUNT <= 255,
					"Texture index is a byte. You will need to change its type if you now have more than 255 blocks."
				);
				const auto tex = static_cast<GLfloat>(chunk.data[begin.x][begin.y] - 2); // TODO: -2 for None and Air. Handle this better.
				buildVBOData.push_back({.pos = origin, .tex = tex});
				buildVBOData.push_back({.pos = origin + glm::vec2{size.x, 0}, .tex = tex});
				buildVBOData.push_back({.pos = origin + size, .tex = tex});
				buildVBOData.push_back({.pos = origin + glm::vec2{0, size.y}, .tex = tex});

				buildEBOData.push_back(vertexCount + 0);
				buildEBOData.push_back(vertexCount + 1);
				buildEBOData.push_back(vertexCount + 2);
				buildEBOData.push_back(vertexCount + 2);
				buildEBOData.push_back(vertexCount + 3);
				buildEBOData.push_back(vertexCount + 0);
			});

			data.mesh.setBufferData(buildVBOData, buildEBOData);

			buildVBOData.clear();
			buildEBOData.clear();
		}

		{ // Physics
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

			greedyExpand([&](const auto& pos, const auto& blockMeta) ENGINE_INLINE {
				return getBlockMeta(chunk.data[pos.x][pos.y]).solid;
			}, [&](const auto& begin, const auto& end) ENGINE_INLINE {
				const auto halfSize = MapChunk::blockSize * 0.5f * Engine::Glue::as<b2Vec2>(end - begin);
				const auto center = MapChunk::blockSize * Engine::Glue::as<b2Vec2>(begin) + halfSize;
				shape.SetAsBox(halfSize.x, halfSize.y, center, 0.0f);
				body.CreateFixture(&fixtureDef);
			});
		}
	}

	void MapSystem::loadChunk(const glm::ivec2 chunkPos, MapChunk& chunk) {
		const auto chunkBlockPos = chunkToBlock(chunkPos);

		for (glm::ivec2 bpos = {0, 0}; bpos.x < MapChunk::size.x; ++bpos.x) {
			for (bpos.y = 0; bpos.y < MapChunk::size.y; ++bpos.y) {
				const auto absPos = chunkBlockPos + bpos;
				chunk.data[bpos.x][bpos.y] = mgen.value(absPos.x, absPos.y);
			}
		}
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

	/*
	const MapChunk* MapSystem::getChunkData(const glm::ivec2 chunk, bool load) {
		const auto region = chunkToRegion(chunk);
		auto found = regions.find(region);

		if (found == regions.cend()) {
			if (load) {
				// TODO: load regions
			} else {
				return nullptr;
			}
		}
		const auto chunkIndex = chunkToRegionIndex(chunk);
		return &found->second->data[chunkIndex.x][chunkIndex.y];
	}*/
}
