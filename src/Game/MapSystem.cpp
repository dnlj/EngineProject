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
		camera = &engine.camera;
		shader = engine.shaderManager.get("shaders/terrain");
		texture = engine.textureManager.get("../assets/test.png");

		for (int x = 0; x < mapSize.x; ++x) {
			for (int y = 0; y < mapSize.y; ++y) {
				chunks[x][y].setup(world, shader.get(), texture.get());
			}
		}

		std::fill(&loadedRegions[0][0], &loadedRegions[0][0] + regionCount.x * regionCount.y, glm::ivec2{0x7FFF'FFFF, 0x7FFF'FFFF});
	}

	void MapSystem::run(float dt) {
		updateOrigin();

		{
			// TODO: We should probably have a buffer around the screen space for this stuff so it has time to load/gen
			// TODO: Handle chunk/region loading in different thread
			
			// As long as screen size < region size we only need to check the four corners
			const auto minRegion = chunkToRegion(blockToChunk(worldToBlock(camera->getWorldScreenBounds().min)));
			const auto maxRegion = chunkToRegion(blockToChunk(worldToBlock(camera->getWorldScreenBounds().max)));
			ensureRegionLoaded(minRegion);
			ensureRegionLoaded(maxRegion);
			ensureRegionLoaded({minRegion.x, maxRegion.y});
			ensureRegionLoaded({maxRegion.x, minRegion.y});
		}

		{ // TODO: Move out of MapSystem. We shouldnt be mixing logic and rendering.
			const auto minChunk = blockToChunk(worldToBlock(camera->getWorldScreenBounds().min));
			const auto maxChunk = blockToChunk(worldToBlock(camera->getWorldScreenBounds().max));
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

	void MapSystem::setValueAt(const glm::vec2 wpos, int value) {
		// TODO: Make conversion functions for all of these? Better names

		const auto blockOffset = glm::floor(wpos / MapChunk::tileSize);
		const auto blockIndex = (MapChunk::size + glm::ivec2{blockOffset} % MapChunk::size) % MapChunk::size;

		//std::cout << "Bidx: " << blockIndex.x << ", " << blockIndex.y << "\n";
		const glm::ivec2 chunkOffset = glm::floor(blockOffset / glm::vec2{MapChunk::size});
		const auto chunkPos = mapOffset + chunkOffset;
		const auto chunkIndex = (mapSize + glm::ivec2{chunkPos} % mapSize) % mapSize;

		auto& chunk = chunks[chunkIndex.x][chunkIndex.y];
		chunk.data[blockIndex.x][blockIndex.y] = value;
		// TODO: we really only want to generate once if we the chunk has been changed since last frame.
		// TODO: as it is now we generate once per edit, which could be many times per frame
		chunk.generate();
	}
	
	glm::ivec2 MapSystem::worldToBlock(const glm::vec2 world) const {
		const glm::ivec2 blockOffset = glm::floor(world / MapChunk::tileSize);
		return getBlockOffset() + blockOffset;
	}

	glm::vec2 MapSystem::blockToWorld(const glm::ivec2 block) const {
		return glm::vec2{block - mapOffset * MapChunk::size} * MapChunk::tileSize;
	}

	glm::ivec2 MapSystem::blockToChunk(const glm::ivec2 block) const {
		// Integer division + floor
		auto d = block / MapChunk::size;
		d.x = d.x * MapChunk::size.x == block.x ? d.x : d.x - (block.x < 0);
		d.y = d.y * MapChunk::size.y == block.y ? d.y : d.y - (block.y < 0);
		return d;
	}

	glm::ivec2 MapSystem::chunkToBlock(const glm::ivec2 chunk) const {
		return chunk * MapChunk::size;
	}

	glm::ivec2 MapSystem::chunkToRegion(const glm::ivec2 chunk) const {
		// Integer division + floor
		auto d = chunk / regionSize;
		d.x = d.x * regionSize.x == chunk.x ? d.x : d.x - (chunk.x < 0);
		d.y = d.y * regionSize.y == chunk.y ? d.y : d.y - (chunk.y < 0);
		return d;
	}

	glm::ivec2 MapSystem::regionToChunk(const glm::ivec2 region) const {
		return region * regionSize;
	}

	glm::ivec2 MapSystem::regionToIndex(const glm::ivec2 region) const {
		return (regionCount + region % regionCount) % regionCount;
	}

	MapChunk& MapSystem::getChunkAt(glm::ivec2 pos) {
		// Wrap index to valid range
		pos = (mapSize + pos % mapSize) % mapSize;
		return chunks[pos.x][pos.y];
	}

	void MapSystem::ensureRegionLoaded(const glm::ivec2 region) {
		// TODO: Consider combining this and loadRegion. We never call one without the other and they could share some data.
		const auto index = regionToIndex(region);

		if (loadedRegions[index.x][index.y] != region) {
			loadRegion(region);
		}
	}

	void MapSystem::loadRegion(const glm::ivec2 region) {
		std::cout << "loadRegion: " << "(" << region.x << ", " << region.y << ")\n";

		const auto regionStart = regionToChunk(region);

		const auto index = regionToIndex(region);
		loadedRegions[index.x][index.y] = region;

		for (int x = 0; x < regionSize.x; ++x) {
			for (int y = 0; y < regionSize.y; ++y) {
				const auto chunk = regionStart + glm::ivec2{x, y};
				loadChunk(chunk);
			}
		}
	}

	void MapSystem::loadChunk(const glm::ivec2 pos) {
		auto& chunk = getChunkAt(pos);
		const auto chunkBlockPos = chunkToBlock(pos);

		for (glm::ivec2 bpos = {0, 0}; bpos.x < MapChunk::size.x; ++bpos.x) {
			for (bpos.y = 0; bpos.y < MapChunk::size.y; ++bpos.y) {
				const auto absPos = chunkBlockPos + bpos;
				int block = 0;
		
				if (0 < mgen.value(absPos.x, absPos.y)) {
					block = 1;
				}
		
				chunk.data[bpos.x][bpos.y] = block;
			}
		}

		chunk.from(blockToWorld(chunkBlockPos));
		chunk.generate();
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
