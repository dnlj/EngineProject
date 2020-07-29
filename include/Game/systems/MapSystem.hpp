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
#include <Engine/EngineInstance.hpp>
#include <Engine/ShaderManager.hpp>
#include <Engine/Graphics/Mesh.hpp>
#include <Engine/Clock.hpp>

// Game
#include <Game/Common.hpp>
#include <Game/EntityFilter.hpp>
#include <Game/MapChunk.hpp>
#include <Game/MapGenerator.hpp>

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
			constexpr static glm::ivec2 size = {16, 16};
			MapChunk data[size.x][size.y];
			std::atomic<int32> loadedChunks = 0;
			Engine::Clock::TimePoint lastUsed;

			bool loading() const {
				// TODO: look into which memory order is needed
				return loadedChunks.load() != size.x * size.y;
			}
			static_assert(decltype(loadedChunks)::is_always_lock_free);
	};

	// TODO: move
	class MapEditComponent {
		public:
			bool place = false;
			bool remove = false;
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
			void ensurePlayAreaLoaded(glm::ivec2 chunkPos);

			// TODO: Name? this isnt consistent with our other usage of offset
			// TODO: Doc. Gets the size of the current offset in blocks coordinates
			glm::ivec2 getBlockOffset() const;
			
			// TODO: Doc
			void setValueAt(const glm::vec2 wpos, int value);

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
			glm::ivec2 blockToChunk(const glm::ivec2 block) const;

			/**
			 * Converts from chunk coordinates to block coordinates.
			 */
			glm::ivec2 chunkToBlock(const glm::ivec2 chunk) const;

			/**
			 * Converts from chunk coordinates to region coordinates.
			 */
			glm::ivec2 chunkToRegion(const glm::ivec2 chunk) const;

			/**
			 * Converts from chunk coordinates to an index wrapped at increments of MapRegion::size.
			 */
			glm::ivec2 chunkToRegionIndex(const glm::ivec2 chunk) const;

			/**
			 * Converts from chunk coordinates to an index wrapped at increments of activeAreaSize.
			 */
			glm::ivec2 chunkToActiveIndex(const glm::ivec2 chunk) const;

			/**
			 * Converts from region coordinates to chunk coordinates.
			 */
			glm::ivec2 regionToChunk(const glm::ivec2 region) const;

			/**
			 * Converts from a region to an index wrapped at increments of regionSize.
			 */
			glm::ivec2 MapSystem::regionToIndex(const glm::ivec2 region) const;

		private:
			// TODO: split?
			struct Vertex {
				glm::vec2 pos;
				//GLuint texture = 0; // TODO: probably doesnt need to be 32bit
			};

			struct ActiveChunkData {
				ActiveChunkData() = default;
				ActiveChunkData(const ActiveChunkData&) = delete;
				ActiveChunkData& operator=(const ActiveChunkData&) = delete;

				glm::ivec2 chunkPos;
				Engine::ECS::Entity ent;
				Engine::Graphics::Mesh mesh; // TODO: move to comp?
			};

			// TODO: Doc
			void buildActiveChunkData(ActiveChunkData& data, const MapChunk& chunk);

			/**
			 * Gets a region if it is already loaded; otherwise queues the region for loading.
			 * @see MapRegion::loading
			 * @see queueRegionToLoad
			 * @see loadChunk
			 */
			MapRegion& ensureRegionLoaded(const glm::ivec2 regionPos);
			
			// TODO: Doc
			void loadChunk(const glm::ivec2 chunkPos, MapChunk& chunk);

			// TODO: Doc
			void loadChunkAsyncWorker();

			// TODO: Doc
			void queueRegionToLoad(glm::ivec2 regionPos, MapRegion& region);

		public: // TODO: make proper accessors if we actually end up needing this stuff
			glm::ivec2 activeAreaOrigin = {0, 0};
			ActiveChunkData activeAreaData[activeAreaSize.x][activeAreaSize.y];
			Engine::Shader shader;
			Engine::Texture texture;

		private:
			std::condition_variable condv;
			std::thread threads[ENGINE_DEBUG ? 8 : 2]; // TODO: Some kind of worker thread pooling in EngineInstance?
			std::mutex chunksToLoadMutex;

			// TODO: C++20: use atomic_flag since it now has a `test` member function.
			std::atomic<bool> threadsShouldExit = false;
			static_assert(decltype(threadsShouldExit)::is_always_lock_free);

			struct Job { // TODO: replace with actual job system in EngineInstance
				glm::ivec2 chunkPos;
				MapRegion& region;
				MapChunk& chunk;
			};
			std::queue<Job> chunksToLoad;

			// TODO: atm we never unload regions
			Engine::FlatHashMap<glm::ivec2, std::unique_ptr<MapRegion>> regions;

			Engine::ECS::Entity mapEntity;
			EntityFilter& playerFilter;

			std::vector<Vertex> buildVBOData;
			std::vector<GLushort> buildEBOData;

			Game::MapGenerator<
				Game::BiomeA,
				Game::BiomeC
			> mgen{12345};
	};
}
