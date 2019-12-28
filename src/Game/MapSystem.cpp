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

		for (int y = 0; y < mapSize.y; ++y) {
			for (int x = 0; x < mapSize.x; ++x) {
				auto& chunk = chunks[x][y];

				chunk.setup(
					world,
					shader.get(),
					texture.get()
				);

				// TODO: switch to regions?
				loadChunk(chunk, glm::ivec2{x, y});
			}
		}
	}

	void MapSystem::run(float dt) {
		updateOrigin();
		const auto minChunk = worldToChunk(camera->getWorldScreenBounds().min);
		const auto maxChunk = worldToChunk(camera->getWorldScreenBounds().max);


		// TODO: this shoudl be before edit
		{
			// TODO: we should probably have a buffer around the screen space for this stuff so it has time to load/gen
			// TODO: Handle chunk/region loading in different thread
			// TODO: if we had velocity we would only need to check two sides instead of all four
			for (int x = minChunk.x; x <= maxChunk.x; ++x) {
				ensureChunkLoaded({x, minChunk.y}).generate();
				ensureChunkLoaded({x, maxChunk.y}).generate();
			}

			for (int y = minChunk.y; y <= maxChunk.y; ++y) {
				ensureChunkLoaded({minChunk.x, y}).generate();
				ensureChunkLoaded({maxChunk.x, y}).generate();
			}
		}

		{
			glm::mat4 mvp = camera->getProjection() * camera->getView();

			for (int y = minChunk.y; y <= maxChunk.y; ++y) {
				for (int x = minChunk.x; x <= maxChunk.x; ++x) {
					getChunkAt({x, y}).draw(mvp);
					getChunkAt({x, y}).draw(mvp * glm::scale(glm::mat4{1.0f}, glm::vec3{0.1f}));
				}
			}
		}
	}
	
	const glm::ivec2& MapSystem::getOffset() const {
		return mapOffset;
	}

	glm::ivec2 MapSystem::worldToChunk(glm::vec2 pos) const {
		// world -> tile
		const auto tile = glm::round(pos / MapChunk::tileSize);

		// tile -> relative chunk
		const glm::ivec2 chunkOffset = glm::floor(tile / glm::vec2{MapChunk::size});

		// relative chunk -> absolute chunk
		return mapOffset * originRange + chunkOffset;
	}

	glm::vec2 MapSystem::chunkToWorld(glm::ivec2 pos) const {
		// absolute -> relative
		pos -= mapOffset * originRange;

		// chunk -> tile
		return glm::vec2{pos * MapChunk::size} * MapChunk::tileSize;
	}

	MapChunk& MapSystem::getChunkAt(glm::ivec2 pos) {
		// Wrap index to valid range
		pos = (mapSize + pos % mapSize) % mapSize;
		return chunks[pos.x][pos.y];
	}

	MapChunk& MapSystem::ensureChunkLoaded(glm::ivec2 pos) {
		auto& chunk = getChunkAt(pos);

		// TODO: Should just store chunk pos on chunk. This is called a lot
		if (worldToChunk(chunk.getPosition()) != pos) {
			loadRegion(chunkToRegion(pos));
		}

		return chunk;
	}

	void MapSystem::loadChunk(MapChunk& chunk, glm::ivec2 pos) {
		chunk.from(chunkToWorld(pos)); // TODO: Data
	}

	glm::ivec2 MapSystem::chunkToRegion(glm::ivec2 pos) {
		// Integer version of floor(a/b)
		return pos / regionSize - glm::ivec2{glm::lessThan(pos, {0, 0})};
	}

	glm::ivec2 MapSystem::regionToChunk(glm::ivec2 region) {
		return region * regionSize;
	}

	void MapSystem::loadRegion(const glm::ivec2 region) {
		std::cout << "loadRegion: " << "(" << region.x << ", " << region.y << ")\n";

		const auto regionStart = regionToChunk(region);
		std::cout << "regionStart: " << "(" << regionStart.x << ", " << regionStart.y << ")\n\n";

		for (int y = 0; y < regionSize.y; ++y) {
			for (int x = 0; x < regionSize.x; ++x) {
				const auto chunk = regionStart + glm::ivec2{x, y};
				getChunkAt(chunk).from(chunkToWorld(chunk)); // TODO: Data
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

			mapOffset.x += static_cast<int>(dir);

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

			mapOffset.y += static_cast<int>(dir);

			// TODO: Figure out a better way to update camera. This seems hacky.
			// TODO: Doing this could also cause problems with other things that use the camera.
			world.getSystem<CameraTrackingSystem>().run(0.0f);
		}
	}
}
