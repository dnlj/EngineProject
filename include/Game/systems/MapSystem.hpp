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
#include <Game/universal.hpp>
#include <Game/MapChunk.hpp>
#include <Game/Connection.hpp>
#include <Game/comps/PhysicsBodyComponent.hpp> // TODO: split physicsbody from componennt
#include <Game/Terrain/TestGenerator.hpp>
#include <Game/Terrain/Generator.hpp>


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

// TODO: New map/terrain:
//       [x] Load terrain. Regions need two more things:
//           - Generation status (added with isChunkLoaded)
//           - Last used - Needed for unloading. Does this go on the chunk? region? Map or Terrain?
//             - I'm thinking probably the region level. That's how we save them so I think that makes sense.
//             - Is this part of the map or terrain system? Idk.
//       [ ] Threading terrain generation
//           - We need a join between each stage since each stage can depend on the last.
//           - Although, we only need that join per request. Assuming no two requests overlap
//             we can still have one main thread for each request and each of those main threads
//             can fork and join per chunk.
//       [ ] Threading active chunk data generation.
//       [x] Unload terrain/regions.
//       [x] Apply edits.
//       [x] Load chunk entities.
//       [x] Store chunk entities.
//       [ ] Portal entities.
//       [ ] Cleanup includes.
//       [ ] (lzjAs29I) Cleanup Terrain/Region/Chunk data access and avoid duplicate calcs.
//           See comments in Terrain functions.
//       [x] Update zone preview.

namespace Game {
	class MapSystem : public System {
		public:
			struct Vertex {
				glm::vec2 pos;
				GLfloat tex;
			};
			static_assert(sizeof(Vertex) == 3*sizeof(GLfloat), "Unexpected vertex size.");

			// TODO: private
			class MapChunkSnapshot { // TODO: move
				public:
					Engine::ECS::Tick tick;
					MapChunk chunk;
			};

			class ActiveChunkData {
				public:
					PhysicsBody body;

					Engine::Gfx::Buffer vbuff;
					Engine::Gfx::Buffer ebuff;
					uint32 ecount;

					/** When was the last time this chunk was used. For unloading old/distant chunks. */
					Engine::Clock::TimePoint lastUsed;

					/** Used to indicate if active data should be rebuilt (if updated == current). */
					Engine::ECS::Tick updated = {};

					/** Cached RLE data for sending to multiple clients. */
					std::vector<byte> rle;

					// TODO: need to serialize for unloaded/inactive chunks. Just a vector<byte> should work?
					std::vector<Engine::ECS::Entity> blockEntities;

					/** The latest confirmed tick for this chunk received from the server. */
					ENGINE_CLIENT_ONLY(Engine::ECS::Tick lastConfirmedTick);

					/** The latest confirmed data for this chunk received from the server. */
					ENGINE_CLIENT_ONLY(MapChunk lastConfimedChunkData);

					/** Unconfirmed client side predicted edits. */
					ENGINE_CLIENT_ONLY(Engine::RingBuffer<MapChunkSnapshot> edits);

					/**
					 * Remove edits up to and including the given tick.
					 * @return True if any edits were removed.
					 */
					ENGINE_CLIENT_ONLY(bool popEditsBefore(Engine::ECS::Tick tick));

					/**
					 * Create a copy of the latest confirmed data plus the predicted edits.
					 */
					ENGINE_CLIENT_ONLY(MapChunk lastWithEdits());
			};

		private:
			std::thread threads[ENGINE_DEBUG ? 8 : 2]; // TODO: Some kind of worker thread pooling in EngineInstance?

			/** The info for chunks */
			Engine::FlatHashMap<UniversalChunkCoord, ActiveChunkData> activeChunks;

			/**
			 * Server side chunk edits. These can be handled much simpler than client side since we
			 * don't need prediction and network correction.
			 */
			ENGINE_SERVER_ONLY(Engine::FlatHashMap<UniversalChunkCoord, MapChunk> serverChunkEdits);

			/** Which chunks have been updated from the server. */
			ENGINE_CLIENT_ONLY(Engine::FlatHashSet<UniversalChunkCoord> chunksUpdatedFromNet);

			/** Which chunks have been updated from client side predicted edits. */
			ENGINE_CLIENT_ONLY(Engine::FlatHashSet<UniversalChunkCoord> chunksUpdatedFromEdits);

			/** Used for sending full RLE chunk updates */
			std::vector<byte> rleTemp;

			// TODO: C++20: use atomic_flag since it now has a `test` member function.
			std::atomic<bool> threadsShouldExit = false;
			static_assert(decltype(threadsShouldExit)::is_always_lock_free);

			Engine::ECS::Entity mapEntity;
			std::vector<Vertex> buildVBOData;
			std::vector<GLushort> buildEBOData;

			Terrain::Terrain terrain;
			ENGINE_SERVER_ONLY(Terrain::TestGenerator testGenerator{terrain, Terrain::TestSeed});
			Engine::FlatHashMap<UniversalRegionCoord, Engine::Clock::TimePoint> regionLastUsed;

		public:
			MapSystem(SystemArg arg);
			~MapSystem();

			void setup();
			void tick();
			void update(float32 dt);
			void network(const NetPlySet plys);

			ENGINE_SERVER_ONLY(Terrain::TestGenerator& generator() noexcept { return testGenerator; });

			void ensurePlayAreaLoaded(Engine::ECS::Entity ply); // TODO: should probably be private

			void chunkFromNet(const Engine::Net::MessageHeader& head, Engine::Net::BufferReader& buff);

			ENGINE_INLINE const auto& getActiveChunks() const noexcept { return activeChunks; }
			ENGINE_INLINE const auto& getTerrain() const noexcept { return terrain; }

		public: // TODO: make proper accessors if we actually end up needing this stuff
			Engine::Gfx::ShaderRef shader;
			Engine::Gfx::Texture2DArray texArr;
			Engine::Gfx::VertexAttributeLayoutRef vertexLayout;

		private:
			// Block connectivity.
			using BCGroupSize = int32;
			constexpr static intz bcInvalidGroup = -1;
			std::vector<BCGroupSize> bcGroups;
			Engine::FlatHashMap<UniversalBlockCoord, intz> bcLookup;
			std::vector<UniversalBlockCoord> bcQueue;

		private:
			/**
			 * @warning Does not lock the terrain. That is up to the caller.
			 */
			void makeEdit(BlockId bid, const ActionComponent& actComp, const PhysicsBodyComponent& physComp);

			void checkBlockConnectivity();

			/**
			 * @warning Does not lock the terrain. That is up to the caller.
			 */
			bool setValueAt(const UniversalBlockCoord blockCoord, BlockId bid);

			// TODO: recycle old bodies?
			PhysicsBody createBody(ZoneId zoneId);

			void buildActiveChunkData(ActiveChunkData& data, const UniversalChunkCoord chunkPos);

			// Server only, still declared here for simplicity.
			void queueGeneration(const Terrain::Request& request);

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
