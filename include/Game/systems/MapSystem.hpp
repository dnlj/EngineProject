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

			constexpr static glm::ivec2 size = {16, 16};
			ChunkInfo data[size.x][size.y];
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

				return loadedChunks.load() != size.x * size.y;
			}
			static_assert(decltype(loadedChunks)::is_always_lock_free);
	};

	class MapSystem : public System {
		public:
			struct Vertex {
				glm::vec2 pos;
				GLfloat tex;
			};
			static_assert(sizeof(Vertex) == 3*sizeof(GLfloat), "Unexpected vertex size.");

		public:
			MapSystem(SystemArg arg);
			~MapSystem();

			void setup();
			void tick();
			void update(float32 dt);
			void ensurePlayAreaLoaded(Engine::ECS::Entity ply); // TODO: should probably be private

			void chunkFromNet(const Engine::Net::MessageHeader& head, Engine::Net::BufferReader& buff);

			// TODO: Doc
			void setValueAt2(const BlockVec blockPos, BlockId bid);

		public: // TODO: make proper accessors if we actually end up needing this stuff
			Engine::Gfx::ShaderRef shader;
			Engine::Gfx::Texture2DArray texArr;

			Engine::Gfx::VertexAttributeLayoutRef vertexLayout;
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

			/** The info for chunks */
			Engine::FlatHashMap<glm::ivec2, ActiveChunkData> activeChunks;
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

			std::vector<Vertex> buildVBOData;
			std::vector<GLushort> buildEBOData;

			MapGenerator2 mgen{12345};

			// TODO: recycle old bodies?
			PhysicsBody createBody(ZoneId zoneId);

			// TODO: Doc
			void buildActiveChunkData(ActiveChunkData& data, glm::ivec2 chunkPos);

			// TODO: Doc
			void loadChunk(const glm::ivec2 chunkPos, MapRegion::ChunkInfo& chunkInfo) const noexcept;

			// TODO: Doc
			void loadChunkAsyncWorker();

			// TODO: Doc
			void queueRegionToLoad(glm::ivec2 regionPos, MapRegion& region);

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
