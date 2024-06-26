#pragma once

// STD
#include <thread>
#include <condition_variable>
#include <queue>
#include <memory>
#include <atomic>

// GLM
#include <glm/vector_relational.hpp>
#include <glm/vec2.hpp>

// Engine
#include <Engine/Clock.hpp>
#include <Engine/ECS/ecs.hpp>
#include <Engine/Gfx/Buffer.hpp>
#include <Engine/Gfx/resources.hpp>
#include <Engine/Gfx/Texture.hpp>
#include <Engine/Gfx/VertexAttributeLayout.hpp>
#include <Engine/ThreadSafeQueue.hpp>

// Game
#include <Game/common.hpp>
#include <Game/MapChunk.hpp>
#include <Game/MapGenerator2.hpp>
#include <Game/Connection.hpp>
#include <Game/comps/PhysicsBodyComponent.hpp> // TODO: split physicsbody from componennt

// TODO: This documentation and comments are likely out of date since the multiplayer and chunk/region/zone reworks.
// TODO: Document the different coordinate systems and terms used here.
// TODO: convert to use sized types - int32, float32, etc.
// TODO: Change all mentions of "tile" to "block". Its a more common term.
// TODO: Standardize terms to be `xyzOffset`, `xyzPosition`, `xyzIndex`

/**
 * World Coordinates - Always relative to Box2D origin. When mapOffset is changed
 * Region - A grouping of chunks. Used for saving/loading.
 *
 * xyzOffset - The position of xyz relative to the current origin shift.
 * xyzPos - The position of xyz in absolute terms. Always the same regardless of the origin shift.
 * xyzIndex - The index of xyz into its storage array.
 *
 */
namespace Game {
	class MapRegion {
		public:
			struct ChunkInfo {
				MapChunk chunk;
				std::vector<BlockEntityDesc> entData;
			};

			ChunkInfo data[regionSize.x][regionSize.y];
			std::atomic<int32> loadedChunks = 0;
			Engine::Clock::TimePoint lastUsed;

			bool loading() const {
				// TODO: look into which memory order is needed

				// TODO: Why is this always false on the client? I assume it was
				// something to do with always loading the chunks on the server?
				//
				// Even if thats the case we should be able to premptevly
				// generate them on the client and then update them once we get
				// the real info from the server?

				if constexpr (ENGINE_CLIENT) {
					return false;
				}

				return loadedChunks.load() != regionSize.x * regionSize.y;
			}
			static_assert(decltype(loadedChunks)::is_always_lock_free);
	};

	// TODO: fmt printers

	class UniversalChunkCoord;
	class UniversalBlockCoord;

	class UniversalRegionCoord {
		public:
			RealmId realmId;
			RegionVec pos;
			constexpr bool operator==(const UniversalRegionCoord&) const noexcept = default;
			ENGINE_INLINE constexpr UniversalChunkCoord toChunk() const noexcept;
	};

	class UniversalChunkCoord {
		public:
			RealmId realmId;
			ChunkVec pos;
			bool operator==(const UniversalChunkCoord&) const noexcept = default;
			constexpr UniversalChunkCoord operator+(const ChunkVec vec) const noexcept { return {realmId, pos + vec}; }
			ENGINE_INLINE constexpr UniversalRegionCoord toRegion() const noexcept { return { realmId, chunkToRegion(pos) }; }
			ENGINE_INLINE constexpr ChunkVec toRegionIndex() const noexcept { return chunkToRegionIndex(pos); }
			ENGINE_INLINE constexpr inline UniversalBlockCoord toBlock() const noexcept;
	};

	class UniversalBlockCoord {
		public:
			RealmId realmId;
			BlockVec pos;

			constexpr UniversalBlockCoord operator+(const BlockVec vec) const noexcept { return {realmId, pos + vec}; }
			constexpr bool operator==(const UniversalBlockCoord&) const noexcept = default;
			ENGINE_INLINE constexpr UniversalChunkCoord toChunk() const noexcept { return { realmId, blockToChunk(pos) }; }
	};

	ENGINE_INLINE constexpr UniversalBlockCoord UniversalChunkCoord::toBlock() const noexcept { return { realmId, chunkToBlock(pos) }; }
	ENGINE_INLINE constexpr UniversalChunkCoord UniversalRegionCoord::toChunk() const noexcept { return { realmId, regionToChunk(pos) }; }
}

template<>
struct Engine::Hash<Game::UniversalRegionCoord> {
	[[nodiscard]]
	size_t operator()(const Game::UniversalRegionCoord& val) const {
		auto seed = hash(val.realmId);
		hashCombine(seed, hash(val.pos));
		return seed;
	}
};

template<>
struct Engine::Hash<Game::UniversalChunkCoord> {
	[[nodiscard]]
	size_t operator()(const Game::UniversalChunkCoord& val) const {
		auto seed = hash(val.realmId);
		hashCombine(seed, hash(val.pos));
		return seed;
	}
};

template<>
struct Engine::Hash<Game::UniversalBlockCoord> {
	[[nodiscard]]
	size_t operator()(const Game::UniversalBlockCoord& val) const {
		auto seed = hash(val.realmId);
		hashCombine(seed, hash(val.pos));
		return seed;
	}
};

namespace Game {
	class MapSystem : public System {
		public:
			struct Vertex {
				glm::vec2 pos;
				GLfloat tex;
			};
			static_assert(sizeof(Vertex) == 3*sizeof(GLfloat), "Unexpected vertex size.");

			struct ActiveChunkData {
				PhysicsBody body;

				Engine::Gfx::Buffer vbuff;
				Engine::Gfx::Buffer ebuff;
				uint32 ecount;

				Engine::Clock::TimePoint lastUsed;
				Engine::ECS::Tick updated = {};
				std::vector<byte> rle;

				// TODO: need to serialize for unloaded/inactive chunks. Just a vector<byte> should work?
				std::vector<Engine::ECS::Entity> blockEntities;
			};

		private:
			std::thread threads[ENGINE_DEBUG ? 8 : 2]; // TODO: Some kind of worker thread pooling in EngineInstance?

			/** The info for chunks */
			Engine::FlatHashMap<UniversalChunkCoord, ActiveChunkData> activeChunks;
			Engine::FlatHashMap<UniversalChunkCoord, MapChunk> chunkEdits;

			/** Used for sending full RLE chunk updates */
			std::vector<byte> rleTemp;

			// TODO: C++20: use atomic_flag since it now has a `test` member function.
			std::atomic<bool> threadsShouldExit = false;
			static_assert(decltype(threadsShouldExit)::is_always_lock_free);

			using Job = std::function<void()>;
			Engine::ThreadSafeQueue<Job> chunkQueue;
			Engine::FlatHashMap<UniversalRegionCoord, std::unique_ptr<MapRegion>> regions;
			Engine::ECS::Entity mapEntity;

			std::vector<Vertex> buildVBOData;
			std::vector<GLushort> buildEBOData;

			MapGenerator2 mgen{12345};

		public:
			MapSystem(SystemArg arg);
			~MapSystem();

			void setup();
			void tick();
			void update(float32 dt);
			void network(const NetPlySet plys);

			void ensurePlayAreaLoaded(Engine::ECS::Entity ply); // TODO: should probably be private

			void chunkFromNet(const Engine::Net::MessageHeader& head, Engine::Net::BufferReader& buff);

			void setValueAt2(const UniversalBlockCoord blockPos, BlockId bid);

			ENGINE_INLINE const auto& getActiveChunks() const noexcept { return activeChunks; }
			ENGINE_INLINE const auto& getLoadedRegions() const noexcept { return regions; }

		public: // TODO: make proper accessors if we actually end up needing this stuff
			Engine::Gfx::ShaderRef shader;
			Engine::Gfx::Texture2DArray texArr;

			Engine::Gfx::VertexAttributeLayoutRef vertexLayout;

		private:
			// TODO: recycle old bodies?
			PhysicsBody createBody(ZoneId zoneId);

			void buildActiveChunkData(ActiveChunkData& data, const UniversalChunkCoord chunkPos);

			void loadChunk(const UniversalChunkCoord chunkPos, MapRegion::ChunkInfo& chunkInfo) const noexcept;

			void loadChunkAsyncWorker();

			void queueRegionToLoad(const UniversalRegionCoord regionPos, MapRegion& region);

			template<BlockEntityType Type>
			Engine::ECS::Entity buildBlockEntity(const BlockEntityDesc& data, const ActiveChunkData& activeChunkData) {
				static_assert(Type != Type, "Missing specialization.");
			}

			template<BlockEntityType Type>
			void storeBlockEntity(BlockEntityTypeData<Type>& data, const Engine::ECS::Entity ent) {
				static_assert(Type != Type, "Missing specialization.");
			}
	};
}
