// GLM
#include <glm/gtc/matrix_transform.hpp>

// Game
#include <Game/MapSystem.hpp>
#include <Game/PhysicsSystem.hpp>
#include <Game/SpriteSystem.hpp>
#include <Game/CameraTrackingSystem.hpp>


namespace Game {
	MapSystem::MapSystem(World& world) : SystemBase{world} {
		priorityAfter = world.getBitsetForSystems<Game::CameraTrackingSystem>();
	}

	void MapSystem::setup(Engine::EngineInstance& engine) {
		input = &engine.inputManager;
		camera = &engine.camera;
		shader = engine.shaderManager.get("shaders/terrain");
		texture = engine.textureManager.get("../assets/test.png");

		for (int x = 0; x < mapSize.x; ++x) {
			for (int y = 0; y < mapSize.y; ++y) {
				chunks[x][y].setup(world, shader.get(), texture.get());
			}
		}
	}

	void MapSystem::run(float dt) {
		updateOrigin();
		const auto minChunk = blockToChunk(worldToBlock(camera->getWorldScreenBounds().min));
		const auto maxChunk = blockToChunk(worldToBlock(camera->getWorldScreenBounds().max));

		// TODO: this shoudl be before edit
		{
			// TODO: we should probably have a buffer around the screen space for this stuff so it has time to load/gen
			// TODO: Handle chunk/region loading in different thread
			// TODO: if we had velocity we would only need to check two sides instead of all four
			
			const auto region = chunkToRegion(minChunk);
			std::cout << "Region: " << region.x << ", " << region.y << "\n";

			for (int x = minChunk.x; x <= maxChunk.x; ++x) {
				ensureChunkLoaded({x, minChunk.y});
				ensureChunkLoaded({x, maxChunk.y});
			}

			for (int y = minChunk.y; y <= maxChunk.y; ++y) {
				ensureChunkLoaded({minChunk.x, y});
				ensureChunkLoaded({maxChunk.x, y});
			}
		}

		{
			glm::mat4 mvp = camera->getProjection() * camera->getView();

			for (int y = minChunk.y; y <= maxChunk.y; ++y) {
				for (int x = minChunk.x; x <= maxChunk.x; ++x) {
					getChunkAt({x, y}).draw(mvp);
					//getChunkAt({x, y}).draw(mvp * glm::scale(glm::mat4{1.0f}, glm::vec3{0.1f}));
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

	void MapSystem::setValueAt(const glm::vec2 wpos) {
		// TODO: Make conversion functions for all of these? Better names

		const auto blockOffset = glm::floor(wpos / MapChunk::tileSize);
		// TODO: there should be a better way of dealing with negatives. Lookinto this.
		const auto blockIndex = (MapChunk::size + glm::ivec2{blockOffset} % MapChunk::size) % MapChunk::size;

		//std::cout << "Bidx: " << blockIndex.x << ", " << blockIndex.y << "\n";
		const glm::ivec2 chunkOffset = glm::floor(blockOffset / glm::vec2{MapChunk::size});
		const auto chunkPos = mapOffset + chunkOffset;
		// TODO: there should be a better way of dealing with negatives. Lookinto this.
		const auto chunkIndex = (mapSize + glm::ivec2{chunkPos} % mapSize) % mapSize;

		auto& chunk = chunks[chunkIndex.x][chunkIndex.y];
		chunk.data[blockIndex.x][blockIndex.y] = 0;
		chunk.generate();
	}

	glm::ivec2 MapSystem::worldToChunk(const glm::vec2 worldPos) const {
		const auto blockOffset = glm::floor(worldPos / MapChunk::tileSize);
		const glm::ivec2 chunkOffset = glm::floor(blockOffset / glm::vec2{MapChunk::size});
		return mapOffset + chunkOffset;
	}

	glm::ivec2 MapSystem::worldToBlock(const glm::vec2 worldPos) const {
		const glm::ivec2 blockOffset = glm::floor(worldPos / MapChunk::tileSize);
		return getBlockOffset() + blockOffset;
	}

	glm::ivec2 MapSystem::blockToChunk(const glm::ivec2 block) const {
		// Integer division + floor 
		const auto d = block / MapChunk::size;
		return d * MapChunk::size == block ? d : d - glm::ivec2{glm::lessThan(block, {0, 0})};
	}

	glm::vec2 MapSystem::chunkToWorld(glm::ivec2 chunkPos) const {
		const auto chunkOffset = chunkPos - mapOffset;
		return glm::vec2{chunkOffset * MapChunk::size} * MapChunk::tileSize;
	}

	glm::ivec2 MapSystem::chunkToBlock(const glm::ivec2 chunkPos) const {
		return chunkPos * MapChunk::size;
	}

	glm::ivec2 MapSystem::chunkToRegion(const glm::ivec2 chunk) {
		// Integer division + floor
		const auto d = chunk / regionSize;
		return d * regionSize == chunk ? d : d - glm::ivec2{glm::lessThan(chunk, {0, 0})};
	}

	glm::ivec2 MapSystem::regionToChunk(glm::ivec2 region) {
		return region * regionSize;
	}

	MapChunk& MapSystem::getChunkAt(glm::ivec2 pos) {
		// Wrap index to valid range
		pos = (mapSize + pos % mapSize) % mapSize;
		return chunks[pos.x][pos.y];
	}

	void MapSystem::ensureRegionLoaded(glm::ivec2 region) {
		// block / chunkSize / regionSize
	}

	MapChunk& MapSystem::ensureChunkLoaded(glm::ivec2 pos) {
		auto& chunk = getChunkAt(pos);

		// TODO: We are checking chunks but loading regions? strange.
		if (chunk.getPosition() != pos) {
			loadRegion(chunkToRegion(pos));
		}

		return chunk;
	}

	void MapSystem::loadChunk(const glm::ivec2 pos) {
		auto& chunk = getChunkAt(pos);
		const auto chunkBlockPos = chunkToBlock(pos);

		for (glm::ivec2 tpos = {0, 0}; tpos.x < MapChunk::size.x; ++tpos.x) {
			for (tpos.y = 0; tpos.y < MapChunk::size.y; ++tpos.y) {
				const auto absPos = chunkBlockPos + tpos;
				auto block = 0;
		
				if (0 < mgen.value(absPos.x, absPos.y)) {
					block = 1;
				}
		
				// chunk.data[tpos.x][tpos.y] = block;
			}
		}

		chunk.from(chunkToWorld(pos), pos);
		chunk.generate();
	}

	void MapSystem::loadRegion(const glm::ivec2 region) {
		std::cout << "loadRegion: " << "(" << region.x << ", " << region.y << ")\n";

		const auto regionStart = regionToChunk(region);
		std::cout << "regionStart: " << "(" << regionStart.x << ", " << regionStart.y << ")\n\n";

		for (int x = 0; x < regionSize.x; ++x) {
			for (int y = 0; y < regionSize.y; ++y) {
				const auto chunk = regionStart + glm::ivec2{x, y};
				loadChunk(chunk);
			}
		}
	}

	void MapSystem::updateOrigin() {
		// TODO: Move to own system. This doesnt really depend on the map.
		const auto& pos = camera->getPosition();
		constexpr auto range = glm::vec2{MapChunk::size * originRange} * MapChunk::tileSize;

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
