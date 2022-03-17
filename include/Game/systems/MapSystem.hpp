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
#include <Engine/ShaderManager.hpp>
#include <Engine/Gfx/Mesh.hpp>
#include <Engine/Clock.hpp>
#include <Engine/ECS/Common.hpp>
#include <Engine/ThreadSafeQueue.hpp>
#include <Engine/Gfx/Texture.hpp>

// Game
#include <Game/Common.hpp>
#include <Game/MapChunk.hpp>
#include <Game/MapGenerator2.hpp>
#include <Game/Connection.hpp>
#include <Game/comps/MapEditComponent.hpp>

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

			constexpr static glm::ivec2 size = {16, 16};
			ChunkInfo data[size.x][size.y];
			std::atomic<int32> loadedChunks = 0;
			Engine::Clock::TimePoint lastUsed;

			bool loading() const {
				// TODO: look into which memory order is needed
				if constexpr (ENGINE_CLIENT) {
					return false;
				}

				return loadedChunks.load() != size.x * size.y;
			}
			static_assert(decltype(loadedChunks)::is_always_lock_free);
	};

	class MapSystem : public System {
		public:
			/** The number of chunks in each region */
			constexpr static glm::ivec2 regionSize = {16, 16};

			/** The number of regions in the map */
			constexpr static glm::ivec2 regionCount = {3, 3};

			/** The number of chunks in the map */
			constexpr static glm::ivec2 mapSize = regionCount * regionSize;

			// TODO: Doc
			constexpr static glm::ivec2 activeAreaSize = {8, 8};
			static_assert(!(activeAreaSize.x & (activeAreaSize.x - 1)), "Must be power of two");
			static_assert(!(activeAreaSize.y & (activeAreaSize.y - 1)), "Must be power of two");

		public:
			MapSystem(SystemArg arg);
			~MapSystem();

			void setup();
			void tick();
			void run(float32 dt);
			void ensurePlayAreaLoaded(Engine::ECS::Entity ply);

			void chunkFromNet(Connection& from, const Engine::Net::MessageHeader& head);

			// TODO: Name? this isnt consistent with our other usage of offset
			// TODO: Doc. Gets the size of the current offset in blocks coordinates
			glm::ivec2 getBlockOffset() const;
			
			// TODO: Doc
			void setValueAt(const glm::vec2 wpos, BlockId bid);

			/**
			 * Converts from world coordinates to block coordinates.
			 */
			glm::ivec2 worldToBlock(const glm::vec2 world) const;

			/**
			 * Converts from block coordinates to world coordinates.
			 */
			glm::vec2 blockToWorld(const glm::ivec2 block) const;

			/**
			 * Converts from block coordinates to chunk coordinates.
			 */
			ENGINE_INLINE constexpr static glm::ivec2 blockToChunk(const glm::ivec2 block) noexcept {
				// Integer division + floor
				auto d = block / MapChunk::size;
				d.x = d.x * MapChunk::size.x == block.x ? d.x : d.x - (block.x < 0);
				d.y = d.y * MapChunk::size.y == block.y ? d.y : d.y - (block.y < 0);
				return d;
			}

			/**
			 * Converts from chunk coordinates to block coordinates.
			 */
			ENGINE_INLINE constexpr static glm::ivec2 chunkToBlock(const glm::ivec2 chunk) noexcept {
				return chunk * MapChunk::size;
			}

			/**
			 * Converts from chunk coordinates to region coordinates.
			 */
			ENGINE_INLINE constexpr static glm::ivec2 chunkToRegion(const glm::ivec2 chunk) noexcept {
				// Integer division + floor
				auto d = chunk / regionSize;
				d.x = d.x * regionSize.x == chunk.x ? d.x : d.x - (chunk.x < 0);
				d.y = d.y * regionSize.y == chunk.y ? d.y : d.y - (chunk.y < 0);
				return d;
			}

			/**
			 * Converts from chunk coordinates to an index wrapped at increments of MapRegion::size.
			 */
			ENGINE_INLINE constexpr static glm::ivec2 chunkToRegionIndex(const glm::ivec2 chunk) noexcept {
				return (MapRegion::size + chunk % MapRegion::size) % MapRegion::size;
			}

			/**
			 * Converts from region coordinates to chunk coordinates.
			 */
			ENGINE_INLINE constexpr static glm::ivec2 regionToChunk(const glm::ivec2 region) noexcept {
				return region * regionSize;
			}

			/**
			 * Converts from a region to an index wrapped at increments of regionSize.
			 */
			ENGINE_INLINE constexpr static glm::ivec2 regionToIndex(const glm::ivec2 region) noexcept {
				return (regionCount + region % regionCount) % regionCount;
			}

			// TODO: how to handle this since chunk data is loaded async?
			//[[nodiscard]]
			//const MapChunk* getChunkData(const glm::ivec2 chunk, bool load = false);

		public: // TODO: make proper accessors if we actually end up needing this stuff
			Engine::ShaderRef shader;
			Engine::Texture2DArray texArr;

			struct TestData { // TODO: rename
				b2Body* body;
				Engine::Gfx::Mesh mesh;
				Engine::Clock::TimePoint lastUsed;
				Engine::ECS::Tick updated = {};
				std::vector<byte> rle;

				// TODO: need to serialize for unloaded/inactive chunks. Just a vector<byte> should work?
				std::vector<Engine::ECS::Entity> blockEntities;
			};

			Engine::FlatHashMap<glm::ivec2, TestData> activeChunks;
			Engine::FlatHashMap<glm::ivec2, MapChunk> chunkEdits;

		private:
			std::thread threads[ENGINE_DEBUG ? 8 : 2]; // TODO: Some kind of worker thread pooling in EngineInstance?

			/** Used for sending full RLE chunk updates */
			std::vector<byte> rleTemp;

			// TODO: C++20: use atomic_flag since it now has a `test` member function.
			std::atomic<bool> threadsShouldExit = false;
			static_assert(decltype(threadsShouldExit)::is_always_lock_free);

			using Job = std::function<void()>;
			Engine::ThreadSafeQueue<Job> chunkQueue;
			Engine::FlatHashMap<glm::ivec2, std::unique_ptr<MapRegion>> regions;
			Engine::ECS::Entity mapEntity;

			struct Vertex {
				glm::vec2 pos;
				GLfloat tex;
			};
			static_assert(sizeof(Vertex) == 3*sizeof(GLfloat), "Unexpected vertex size.");

			std::vector<Vertex> buildVBOData;
			std::vector<GLushort> buildEBOData;

			MapGenerator2 mgen{12345};

			// TODO: recycle old bodies?
			b2Body* createBody();

			void setupMesh(Engine::Gfx::Mesh& mesh) const;

			// TODO: Doc
			void buildActiveChunkData(TestData& data, glm::ivec2 chunkPos);

			// TODO: Doc
			void loadChunk(const glm::ivec2 chunkPos, MapRegion::ChunkInfo& chunkInfo) const noexcept;

			// TODO: Doc
			void loadChunkAsyncWorker();

			// TODO: Doc
			void queueRegionToLoad(glm::ivec2 regionPos, MapRegion& region);

			template<BlockEntityType Type>
			Engine::ECS::Entity buildBlockEntity(const BlockEntityDesc& data) {
				static_assert(Type != Type, "Missing specialization.");
			}

			template<BlockEntityType Type>
			void storeBlockEntity(BlockEntityTypeData<Type>& data, const Engine::ECS::Entity ent) {
				static_assert(Type != Type, "Missing specialization.");
			}
	};
}
