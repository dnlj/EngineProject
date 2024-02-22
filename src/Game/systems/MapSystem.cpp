// GLM
#include <glm/gtc/matrix_transform.hpp>

// Engine
#include <Engine/Glue/Box2D.hpp>
#include <Engine/Glue/glm.hpp>
#include <Engine/Gfx/ShaderManager.hpp>
#include <Engine/Gfx/ResourceContext.hpp>

// Game
#include <Game/comps/ActionComponent.hpp>
#include <Game/comps/BlockEntityComponent.hpp>
#include <Game/comps/MapAreaComponent.hpp>
#include <Game/comps/PhysicsBodyComponent.hpp>
#include <Game/comps/PhysicsInterpComponent.hpp>
#include <Game/comps/SpriteComponent.hpp>
#include <Game/systems/MapSystem.hpp>
#include <Game/systems/NetworkingSystem.hpp>
#include <Game/systems/ZoneManagementSystem.hpp>
#include <Game/World.hpp>


namespace {
	using namespace Game;
	using Engine::ECS::Entity;
	using Engine::Net::MessageHeader;
	using Engine::Net::BufferReader;

	using PlayerFilter = Engine::ECS::EntityFilterList<
		PlayerFlag,
		PhysicsBodyComponent
	>;

	void recv_MAP_CHUNK(EngineInstance& engine, ConnectionInfo& from, const MessageHeader head, BufferReader& msg) {
		auto& world = engine.getWorld();
		world.getSystem<MapSystem>().chunkFromNet(head, msg);
	}
}


#define BUILD_BLOCK_ENTITY(Type) template<> Engine::ECS::Entity MapSystem::buildBlockEntity<BlockEntityType::Type>(const BlockEntityDesc& desc, const ActiveChunkData& activeChunkData)
#define STORE_BLOCK_ENTITY(Type) template<> void MapSystem::storeBlockEntity<BlockEntityType::Type>(BlockEntityTypeData<BlockEntityType::Type>& data, const Engine::ECS::Entity ent)

namespace Game {
	BUILD_BLOCK_ENTITY(None) {
		return Engine::ECS::INVALID_ENTITY;
	}

	STORE_BLOCK_ENTITY(None) {
		ENGINE_WARN("TODO: Store None"); // TODO: impl
	}

	BUILD_BLOCK_ENTITY(Tree) {
		b2BodyDef bodyDef;
		bodyDef.type = b2_staticBody;
		bodyDef.fixedRotation = true;
		bodyDef.linearDamping = 10.0f;

		b2FixtureDef fixtureDef;
		//fixtureDef.density = 1.0f;
		// Disable collision
		fixtureDef.filter.categoryBits = PhysicsSystem::getCategoryBits(PhysicsCategory::Decoration);
		fixtureDef.filter.maskBits = PhysicsSystem::getMaskBits(PhysicsCategory::Decoration);

		const auto zoneOffset = world.getSystem<ZoneManagementSystem>().getZone(activeChunkData.body.getZoneId()).offset;
		const b2Vec2 pos = Engine::Glue::as<b2Vec2>(blockToWorld2(desc.pos, zoneOffset));
		const auto ent = world.createEntity();

		world.addComponent<NetworkedFlag>(ent);
		
		constexpr const char* texStr[] = {
			"assets/tree1.png",
			"assets/tree2.png",
			"assets/tree3.png",
		};

		int texIdx = desc.data.asTree.type;
		if (texIdx >= std::size(texStr)) {
			ENGINE_WARN("Attempting to create tree with invalid type.");
			texIdx = 0;
		}

		auto& spriteComp = world.addComponent<SpriteComponent>(ent);
		spriteComp.path = texStr[texIdx];
		spriteComp.texture = engine.getTextureLoader().get2D(spriteComp.path);
		spriteComp.layer = RenderLayer::Background;
		{
			const auto& sz = spriteComp.texture.get()->size;
			spriteComp.scale = {pixelsPerBlock, pixelsPerBlock};
			spriteComp.position.x = blockSize * 0.5f;
			spriteComp.position.y = sz.y * (0.5f / pixelsPerMeter);
		}

		//
		//
		//
		//
		//
		// TODO: Should physics comp take a PhysicsBody in constructor? seems like it
		//
		//
		//
		//
		//

		{
			bodyDef.position = pos;
			auto& physSys = world.getSystem<PhysicsSystem>();
			auto& physComp = world.addComponent<PhysicsBodyComponent>(ent, physSys.createBody(ent, bodyDef, activeChunkData.body.getZoneId()));

			b2PolygonShape shape;
			const b2Vec2 tsize = 0.5f * blockSize * Engine::Glue::as<b2Vec2>(desc.data.asTree.size);
			shape.SetAsBox(tsize.x, tsize.y, {0.5f * blockSize, tsize.y}, 0);
			fixtureDef.shape = &shape;
			physComp.createFixture(fixtureDef);
		}

		auto& physInterpComp = world.addComponent<PhysicsInterpComponent>(ent);
		physInterpComp.trans.p = pos;

		return ent;
	}

	STORE_BLOCK_ENTITY(Tree) {
		ENGINE_WARN("TODO: Store Tree"); // TODO: impl
		data.type = 1;
		data.size = {1,20};
	}

	BUILD_BLOCK_ENTITY(Portal) {
		return Engine::ECS::INVALID_ENTITY;
	}

	STORE_BLOCK_ENTITY(Portal) {
		ENGINE_WARN("TODO: Store Portal"); // TODO: impl
	}
}


#undef BUILD_BLOCK_ENTITY
#undef STORE_BLOCK_ENTITY


namespace Game {
	MapSystem::MapSystem(SystemArg arg)
		: System{arg} {
		static_assert(World::orderAfter<MapSystem, CameraTrackingSystem>());
		namespace Gfx = Engine::Gfx;

		auto& rctx = engine.getGraphicsResourceContext();
		vertexLayout = rctx.vertexLayoutManager.create(Gfx::VertexAttributeLayoutDesc{
			{
				{.binding = 0, .divisor = 0}
			},{
				{0, 2, Gfx::NumberType::Float32, Gfx::VertexAttribTarget::Float, false, offsetof(Game::MapSystem::Vertex, pos), 0},
				{1, 1, Gfx::NumberType::Float32, Gfx::VertexAttribTarget::Float, false, offsetof(Game::MapSystem::Vertex, tex), 0}
			}
		});

		for (auto& t : threads) {
			t = std::thread{&MapSystem::loadChunkAsyncWorker, this};
		}
	}

	MapSystem::~MapSystem() {
		threadsShouldExit = true;
		chunkQueue.notify();

		for (auto& t : threads) {
			t.join();
		}
	}

	void MapSystem::setup() {
		using namespace Engine::Gfx;

		{
			auto& netSys = world.getSystem<NetworkingSystem>();
			netSys.setMessageHandler(MessageType::MAP_CHUNK, recv_MAP_CHUNK);
		}

		mapEntity = world.createEntity();
		shader = engine.getShaderLoader().get("shaders/terrain");

		constexpr int32 offset = 2; // offset by 2 to skip None and Air
		const char* textures[BlockId::_count - offset] = {};
		for (int32 i = offset; i < BlockId::_count; ++i) {
			textures[i - offset] = getBlockMeta(static_cast<BlockId>(i)).path;
		}

		// TODO: really no reason to use RGBA here. we dont use alpha
		texArr.setStorage(TextureFormat::SRGBA8, {8, 8, std::size(textures)});
		texArr.setFilter(TextureFilter::Nearest);

		Image img;
		for (int i = 0; auto path : textures) {
			img = path;
			img.flipY();
			texArr.setSubImage(0, {0, 0, i++}, {img.size(), 1}, img);
		}
	}

	PhysicsBody MapSystem::createBody(ZoneId zoneId) {
		b2BodyDef bodyDef;
		bodyDef.type = b2_staticBody;
		bodyDef.awake = false;
		bodyDef.fixedRotation = true;
		return world.getSystem<PhysicsSystem>().createBody(mapEntity, bodyDef, zoneId);
	}

	void MapSystem::tick() {
		const auto currTick = world.getTick();

		const auto makeEdit = [&](BlockId bid, const ActionComponent& actComp, const PhysicsBodyComponent& physComp) {
			const auto& zoneSys = world.getSystem<ZoneManagementSystem>();
			for (int x = -2; x < 3; ++x) {
				for (int y = -2; y < 3; ++y) {
					const auto plyPos = physComp.getPosition();
					//const auto plyBlockPos = BlockVec{plyPos.x, plyPos.y} + ;
					//const BlockVec target = actComp.getTarget() * blockSize + glm::vec2{plyPos.x, plyPos.y};
					const WorldVec placementOffset = {x*blockSize, y*blockSize};
					const BlockVec target = worldToBlock2(
						WorldVec{plyPos.x, plyPos.y} + actComp.getTarget() + placementOffset,
						zoneSys.getZone(physComp.getZoneId()).offset
					);
					setValueAt2(target, bid);
				}
			}
		};

		for (auto& ply : world.getFilter<PlayerFilter>()) {
			const auto& actComp = world.getComponent<ActionComponent>(ply);

			if (actComp.getAction(Action::Attack1).latest) {
				const auto& physBodyComp = world.getComponent<PhysicsBodyComponent>(ply);
				makeEdit(BlockId::Dirt, actComp, physBodyComp);
			}
			if (actComp.getAction(Action::Attack2).latest) {
				const auto& physBodyComp = world.getComponent<PhysicsBodyComponent>(ply);
				makeEdit(BlockId::Air, actComp, physBodyComp);
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
				// I think we could hit this if we get a chunk from the network before we have that area loaded on the client.
				// TODO: Would it be better to just have it load that area here instead of trying to pre-load on the client?
				ENGINE_WARN("Attempting to edit unloaded chunk/region");
				continue;
			}

			const auto chunkIndex = chunkToRegionIndex(chunkPos);
			auto& chunk = regionIt->second->data[chunkIndex.x][chunkIndex.y].chunk;

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

	void MapSystem::chunkFromNet(const Engine::Net::MessageHeader& head, Engine::Net::BufferReader& buff) {
		const byte* begin = reinterpret_cast<const byte*>(buff.read(head.size));
		const byte* end = begin + head.size;
		glm::ivec2 chunkPos;

		memcpy(&chunkPos.x, begin, sizeof(chunkPos.x));
		begin += sizeof(chunkPos.x);
		
		memcpy(&chunkPos.y, begin, sizeof(chunkPos.y));
		begin += sizeof(chunkPos.y);

		// ENGINE_INFO("Recv chunk from net: ", chunkPos.x, " ", chunkPos.y, " ", head.size);
		chunkEdits[chunkPos].fromRLE(begin, end);
	}

	void MapSystem::update(float32 dt) {
		const auto tick = world.getTick();
		auto timeout = world.getTickTime() - std::chrono::seconds{10}; // TODO: how long? 30s?
		auto& netSys = world.getSystem<NetworkingSystem>();

		for (auto ent : world.getFilter<MapAreaComponent>()) {
			auto& mapAreaComp = world.getComponent<MapAreaComponent>(ent);
			const auto begin = mapAreaComp.updates.begin();
			const auto end = mapAreaComp.updates.end();

			auto* conn = netSys.getConnection(ent);
			if (!conn) {
				ENGINE_WARN("Unable to get network connection for entity. This is a bug.");
				return;
			}

			for (auto it = begin; it != end;) {
				const auto& chunkPos = it->first;
				auto& meta = it->second;

				if (meta.tick != tick) {
					// ENGINE_LOG("Remove: ", chunkPos.x, " ", chunkPos.y);
					it = mapAreaComp.updates.erase(it);
					continue;
				}
				
				const auto found = activeChunks.find(chunkPos);
				if (found == activeChunks.end()) { continue; }
				auto& activeData = found->second;

				if (meta.last != activeData.updated) {
					std::vector<byte>* rle = nullptr;

					if (meta.last == 0) { // Fresh chunk
						const auto regionPos = chunkToRegion(chunkPos);
						const auto regionIt = regions.find(regionPos);
						if (regionIt != regions.end() && !regionIt->second->loading()) {
							const auto chunkIndex = chunkToRegionIndex(chunkPos);
							auto& chunkInfo = regionIt->second->data[chunkIndex.x][chunkIndex.y];
							rleTemp.clear();
							chunkInfo.chunk.toRLE(rleTemp);
							rle = &rleTemp;
							//ENGINE_INFO("Send chunk (fresh): ", tick, " ", chunkPos.x, " ", chunkPos.y, " ", size);
						}
					} else if (activeData.rle.empty()) {
						// TODO: i dont think this case should be hit?
						ENGINE_WARN("No RLE data for chunk");
						ENGINE_DEBUG_BREAK;
					} else { // Chunk edit
						rle = &activeData.rle;
						//ENGINE_INFO("Send chunk (edit): ", tick, " ", chunkPos.x, " ", chunkPos.y, " ", size);
					}

					if (auto msg = conn->beginMessage<MessageType::MAP_CHUNK>()) {
						meta.last = activeData.updated;
						const auto size = static_cast<int32>(rle->size() * sizeof(rle->front()));
						byte* data = reinterpret_cast<byte*>(rle->data());
						memcpy(data, &chunkPos.x, sizeof(chunkPos.x));
						memcpy(data + sizeof(chunkPos.x), &chunkPos.y, sizeof(chunkPos.y));
						msg.writeBlob(data, size);
					} else {
						//ENGINE_WARN("Unable to begin MAP_CHUNK message.");
					}
				}

				++it;
			}
		}

		// TODO: Shouldn't this unload logic be in tick instead of update?

		// Unload active chunks
		for (auto it = activeChunks.begin(); it != activeChunks.end();) {
			if (it->second.lastUsed < timeout) {
				// ENGINE_LOG("Unloading chunk: ", it->first.x, ", ", it->first.y);

				// Store block entities
				if constexpr (ENGINE_SERVER) {
					const auto regionPos = chunkToRegion(it->first);
					const auto regionIt = regions.find(regionPos);
					if (regionIt == regions.end() || regionIt->second->loading()) {
						ENGINE_WARN("Attempting to unload a active chunk into unloaded region.");
					} else {
						auto& region = *regionIt->second;
						const auto chunkIndex = chunkToRegionIndex(it->first);
						auto& chunkData = region.data[chunkIndex.x][chunkIndex.y];
						chunkData.entData.clear();

						for (const auto ent : it->second.blockEntities) {
							auto& desc = chunkData.entData.emplace_back();
							const auto& beComp = world.getComponent<BlockEntityComponent>(ent);
							desc.data.type = beComp.type;
							desc.pos = beComp.block;
							desc.data.with([&]<auto Type>(auto& data){
								storeBlockEntity<Type>(data, ent);
							});
							world.deferedDestroyEntity(ent);
						}
					}
				}

				world.getSystem<PhysicsSystem>().destroyBody(it->second.body.takeOwnership()); // TODO: destory body should just work on PhysicsBody instead of b2Body
				it = activeChunks.erase(it);
			} else {
				++it;
			}
		}

		// Unload regions
		for (auto it = regions.begin(); it != regions.end();) {
			if (it->second->lastUsed < timeout && !it->second->loading()) {
				ENGINE_LOG("Unloading region: ", it->first.x, ", ", it->first.y);
				it = regions.erase(it);
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
		const auto& zoneSys = world.getSystem<ZoneManagementSystem>();
		const auto& physComp = world.getComponent<PhysicsBodyComponent>(ply);
		const auto plyPos = Engine::Glue::as<glm::vec2>(physComp.getPosition());
		const auto plyZoneId = physComp.getZoneId();
		const auto plyZoneOffset = zoneSys.getZone(plyZoneId).offset;
		const auto blockPos = worldToBlock2(plyPos, plyZoneOffset);

		// How large of an area to load around the chunk blockPos is in.
		constexpr auto halfAreaSize = ChunkVec{5, 5};

		// How large of a buffer around halfAreaSize to wait befor unloading areas.
		// We want a larger unload area so that we arent constantly loading/unloading
		// when a player is near a chunk border.
		constexpr auto halfBuffSize = ChunkVec{7, 7};

		const auto minAreaChunk = blockToChunk(blockPos) - halfAreaSize;
		const auto maxAreaChunk = blockToChunk(blockPos) + halfAreaSize;
		const auto minBuffChunk = minAreaChunk - halfBuffSize;
		const auto maxBuffChunk = maxAreaChunk + halfBuffSize;

		for (auto chunkPos = minBuffChunk; chunkPos.x <= maxBuffChunk.x; ++chunkPos.x) {
			for (chunkPos.y = minBuffChunk.y; chunkPos.y <= maxBuffChunk.y; ++chunkPos.y) {
				const auto isBufferChunk = chunkPos.x < minAreaChunk.x
					|| chunkPos.x > maxAreaChunk.x
					|| chunkPos.y < minAreaChunk.y
					|| chunkPos.y > maxAreaChunk.y;

				const auto regionPos = chunkToRegion(chunkPos);
				auto regionIt = regions.find(regionPos);

				// Create a new region if needed
				if (regionIt == regions.end()) {
					if (!isBufferChunk) {
						//const auto it = regions.emplace(regionPos, new MapRegion{
						//	.lastUsed = world.getTickTime(),
						//}).first;
						// 
						// Work around for MSVC compiler heap bug.
						// If we use the above RAM usage hits +8GB (runs out of memory, error).
						// With below it hits +2GB at most.
						auto ptr = std::make_unique<MapRegion>();
						ptr->lastUsed = world.getTickTime();
						const auto it = regions.emplace(regionPos, std::move(ptr)).first;

						if constexpr (ENGINE_SERVER) {
							queueRegionToLoad(regionPos, *it->second);
						}
					}
					continue;
				}

				// Update chunk usage info
				const auto& region = regionIt->second;
				region->lastUsed = world.getTickTime();
				if (region->loading()) { continue; }

				#if ENGINE_SERVER
				{
					const auto found = mapAreaComp.updates.contains(chunkPos);
					if (!isBufferChunk || found) {
						mapAreaComp.updates[chunkPos].tick = tick;
					}
				}
				#endif

				// Create active chunk data.
				// The generation of the actual graphics/physics is done in
				// MapSystem::tick based on the last chunk update tick. 
				auto it = activeChunks.find(chunkPos);
				if (it == activeChunks.end()) {
					if (isBufferChunk) { continue; }

					// TODO (QmIupKgJ): we really should probably have a cache of inactive chunks that we can reuse instead of constant destroy/create.
					it = activeChunks.try_emplace(chunkPos).first;
					it->second.body = createBody(plyZoneId);
					ENGINE_DEBUG_ASSERT(it->second.body.valid());
					//ENGINE_INFO2("Make active {}, {}", chunkPos, plyZoneId);

					const auto chunkIndex = chunkToRegionIndex(chunkPos);
					auto& chunkInfo = region->data[chunkIndex.x][chunkIndex.y];

					// Build chunk entities
					if constexpr (ENGINE_SERVER) {
						for (const auto& desc : chunkInfo.entData) {
							Engine::ECS::Entity ent;

							desc.data.with([&]<auto Type>(auto& data) ENGINE_INLINE {
								ent = buildBlockEntity<Type>(desc, it->second);
								if (ent != Engine::ECS::INVALID_ENTITY) {
									auto& beComp = world.addComponent<BlockEntityComponent>(ent);
									beComp.type = desc.data.type;
									beComp.block = desc.pos;
									it->second.blockEntities.push_back(ent);
								} else {
									ENGINE_WARN("Attempting to create invalid block entity.");
								}
							});
						}
					}

					// ENGINE_LOG("Activating chunk: ", chunkPos.x, ", ", chunkPos.y, " (", (it->second.updated == tick) ? "fresh" : "stale", ")");
					it->second.updated = tick;
				} else if (!isBufferChunk) {
					// Only move the non-buffer chunks to avoid any stutter when moving
					// between zones. The buffer chunks will be moved later if the player
					// gets in range again.
					//
					// There still is a small amount of stutter when changing zones in debug
					// mode but its pretty minor and not present at all in release mode. If
					// this turns into an issue in the future the next steps would be to
					// reduce the active area size above (halfAreaSize). At the time of writing
					// this its (5,5) = (5+1+5)^2 = 121 chunks. Really that could probably
					// be reduced to (3,3) = (3+1+3)^2 = 49 which would be a huge
					// improvement. Even just (4,4) = 81 might be enough.
					//
					// Reducing the buffer area size (halfBuffSize) shouldn't have any effect
					// since we only update the nonbuffered chunks immediately.
					
					// If the chunk is already loaded make sure the chunk and its entities are in the correct zone.
					auto& body = it->second.body;
					if (body.getZoneId() != plyZoneId) {
						//ENGINE_INFO2("Moving chunk {} from zone {} to {} @ {}", chunkPos, body.getZoneId(), plyZoneId, tick);
						const auto pos = blockToWorld2(chunkToBlock(chunkPos), plyZoneOffset);
						body.setPosition({pos.x, pos.y});
						body.setZone(plyZoneId);

						// Client side this is handled through the entity networking system.
						if constexpr (ENGINE_SERVER) {
							for (const auto ent : it->second.blockEntities) {
								auto& entPhysComp = world.getComponent<PhysicsBodyComponent>(ent);
								
								// Some entities may have already been moved in the zone system so we need to check the zone here.
								if (entPhysComp.getZoneId() != plyZoneId) {
									const auto oldZoneOffset = zoneSys.getZone(entPhysComp.getZoneId()).offset;
									entPhysComp.moveZone(oldZoneOffset, plyZoneId, plyZoneOffset);
								}
							}
						}
					}
				}
				
				it->second.lastUsed = world.getTickTime();
			}
		}
	}

	void MapSystem::setValueAt2(const BlockVec blockPos, BlockId bid) {
		const auto blockIndex = (MapChunk::size + blockPos % MapChunk::size) % MapChunk::size;
		auto& edit = chunkEdits[blockToChunk(blockPos)];
		edit.data[blockIndex.x][blockIndex.y] = bid;
	}
	
	// TODO: thread this. Not sure how nice box2d will play with it.
	void MapSystem::buildActiveChunkData(ActiveChunkData& data, glm::ivec2 chunkPos) {
		const auto regionPos = chunkToRegion(chunkPos);
		const auto regionIt = regions.find(regionPos);
		if (regionIt == regions.end() || regionIt->second->loading()) [[unlikely]] { return; }
				
		const auto chunkIndex = chunkToRegionIndex(chunkPos);
		auto& chunkInfo = regionIt->second->data[chunkIndex.x][chunkIndex.y];

		if constexpr (ENGINE_SERVER) { // Build edits
			const auto found = chunkEdits.find(chunkPos);
			if (found == chunkEdits.end()) {
				data.rle.clear();
			} else {
				found->second.toRLE(data.rle);
			}
		}

		decltype(auto) greedyExpand = [&chunkInfo](auto usable, auto submitArea) ENGINE_INLINE {
			bool used[MapChunk::size.x][MapChunk::size.y] = {};
			
			for (glm::ivec2 begin = {0, 0}; begin.x < MapChunk::size.x; ++begin.x) {  
				for (begin.y = 0; begin.y < MapChunk::size.y;) {
					const auto& blockMeta = getBlockMeta(chunkInfo.chunk.data[begin.x][begin.y]);
					auto end = begin;
					while (end.y < MapChunk::size.y && !used[end.x][end.y] && usable(end, blockMeta)) { ++end.y; }
					if (end.y == begin.y) { ++begin.y; continue; }

					for (bool cond = true; cond;) {
						//std::fill(&used[end.x][begin.y], &used[end.x][end.y], true);
						memset(&used[end.x][begin.y], 1, end.y - begin.y);
						++end.x;

						if (end.x == MapChunk::size.x) { break; }
						for (int y = begin.y; y < end.y; ++y) {
							if (used[end.x][y] || !usable(glm::ivec2{end.x, y}, blockMeta)) { cond = false; break; }
						}
					}

					submitArea(begin, end);
					begin.y = end.y;
				}
			}
		};

		{ // Render
			// Generate VBO+EBO data
			greedyExpand([&](const auto& pos, const auto& blockMeta) ENGINE_INLINE {
				return blockMeta.id != BlockId::None
					&& blockMeta.id != BlockId::Air
					&& chunkInfo.chunk.data[pos.x][pos.y] == blockMeta.id;
			}, [&](const auto& begin, const auto& end) ENGINE_INLINE {
				// Add buffer data
				glm::vec2 origin = glm::vec2{begin} * blockSize;
				glm::vec2 size = glm::vec2{end - begin} * blockSize;
				const auto vertexCount = static_cast<GLushort>(buildVBOData.size());

				static_assert(BlockId::_count <= 255,
					"Texture index is a byte. You will need to change its type if you now have more than 255 blocks."
				);
				const auto tex = static_cast<GLfloat>(chunkInfo.chunk.data[begin.x][begin.y] - 2); // TODO: -2 for None and Air. Handle this better.
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

			{ // Build vertex buffer
				const auto sz = buildVBOData.size() * sizeof(buildVBOData[0]);
				if (data.vbuff.size() < sz) {
					const auto cap = sz + (sz >> 1);
					data.vbuff.alloc(cap, Engine::Gfx::StorageFlag::DynamicStorage);
				}

				if (sz) {
					data.vbuff.setData(buildVBOData);
				}
			}

			{ // Build element buffer
				const auto count = buildEBOData.size();
				const auto sz = count * sizeof(buildEBOData[0]);
				if (data.ebuff.size() < sz) {
					const auto cap = sz + (sz >> 1);
					data.ebuff.alloc(cap, Engine::Gfx::StorageFlag::DynamicStorage);
				}

				if (sz) {
					data.ebuff.setData(buildEBOData);
				}
				data.ecount = static_cast<uint32>(count);
			}

			buildVBOData.clear();
			buildEBOData.clear();
		}

		{ // Physics
			auto& body = data.body;
			const auto& zoneSys = world.getSystem<ZoneManagementSystem>();
			const auto pos = Engine::Glue::as<b2Vec2>(blockToWorld2(chunkToBlock(chunkPos), zoneSys.getZone(body.getZoneId()).offset));

			// TODO: Look into edge and chain shapes
			// Clear all fixtures
			body.clear();
			body.setPosition(pos);

			b2PolygonShape shape{};
			b2FixtureDef fixtureDef{};
			fixtureDef.shape = &shape;

			greedyExpand([&](const auto& pos, const auto& blockMeta) ENGINE_INLINE {
				return getBlockMeta(chunkInfo.chunk.data[pos.x][pos.y]).solid;
			}, [&](const auto& begin, const auto& end) ENGINE_INLINE {
				const auto halfSize = blockSize * 0.5f * Engine::Glue::as<b2Vec2>(end - begin);
				const auto center = blockSize * Engine::Glue::as<b2Vec2>(begin) + halfSize;
				shape.SetAsBox(halfSize.x, halfSize.y, center, 0.0f);
				body.createFixture(fixtureDef);
			});
		}
	}

	void MapSystem::loadChunk(const glm::ivec2 chunkPos, MapRegion::ChunkInfo& chunkInfo) const noexcept {
		const auto chunkBlockPos = chunkToBlock(chunkPos);
		mgen.init(chunkBlockPos, chunkInfo.chunk, chunkInfo.entData);
	}

	void MapSystem::loadChunkAsyncWorker() {
		Job job;
		while (!threadsShouldExit) {
			if (chunkQueue.popOrWait(job)) {
				job();
			}
		}
	}

	void MapSystem::queueRegionToLoad(glm::ivec2 regionPos, MapRegion& region) {
		std::cout << "Queue region: " << regionPos.x << " " << regionPos.y << "\n";
		const auto regionStart = regionToChunk(regionPos);

		auto lock = chunkQueue.lock();
		for (RegionUnit x = 0; x < regionSize.x; ++x) {
			for (RegionUnit y = 0; y < regionSize.y; ++y) {
				// TODO: maybe have each thread do a whole row of a region? per chunk seems to granular
				chunkQueue.unsafeEmplace([this,
						chunkPos = regionStart + RegionVec{x, y},
						&region,
						&chunkInfo = region.data[x][y]
					] {
					loadChunk(chunkPos, chunkInfo);
					++region.loadedChunks;
				});
			}
		}
		lock.unlock();
		chunkQueue.notify();
	}
}
