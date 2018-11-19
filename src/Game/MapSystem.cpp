// GLM
#include <glm/gtc/matrix_transform.hpp>

// Game
#include <Game/MapSystem.hpp>
#include <Game/PhysicsSystem.hpp>
#include <Game/SpriteSystem.hpp>

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

				loadChunk(chunk, glm::ivec2{x, y});
			}
		}
	}

	void MapSystem::run(float dt) {
		const auto applyEdit = [&](auto func){
			const auto mpos = camera->screenToWorld(input->getMousePosition());

			// Offset of tile relative to current offset
			const auto offsetTile = glm::floor(mpos / MapChunk::tileSize);

			// Chunk position relative to current offset
			const glm::ivec2 offsetChunk = glm::floor(offsetTile / glm::vec2{MapChunk::size});

			// Absolute chunk position
			const auto absChunk = mapOffset * originRange + offsetChunk;

			// Index for this chunk
			const auto indexChunk = (mapSize + absChunk % mapSize) % mapSize;

			// Index of this tile in this chunk
			const auto indexTile = (MapChunk::size + glm::ivec2{offsetTile} % MapChunk::size) % MapChunk::size;

			(chunks[indexChunk.x][indexChunk.y].*func)(indexTile.x, indexTile.y, world.getSystem<PhysicsSystem>());
		};

		if (input->isPressed("edit_place")) {
			applyEdit(&MapChunk::addTile);
		} else if (input->isPressed("edit_remove")) {
			applyEdit(&MapChunk::removeTile);
		}

		
		const auto tlChunk = worldToChunk(camera->screenToWorld({0, 0}));
		const auto brChunk = worldToChunk(camera->screenToWorld(camera->getScreenSize()));


		// TODO: this shoudl be before edit
		{
			// TODO: if we had velocity we would only need to check two sides instead of all four
			for (int x = tlChunk.x; x <= brChunk.x; ++x) {
				loadChunk({x, tlChunk.y});
				loadChunk({x, brChunk.y});
			}

			for (int y = brChunk.y; y <= tlChunk.y; ++y) {
				loadChunk({tlChunk.x, y});
				loadChunk({brChunk.x, y});
			}
		}

		updateOrigin();

		{
			glm::mat4 mvp = camera->getProjection() * camera->getView();

			for (int y = brChunk.y; y <= tlChunk.y; ++y) {
				for (int x = tlChunk.x; x <= brChunk.x; ++x) {
					getChunkAt({x, y}).draw(mvp);
				}
			}
		}
	}
	
	const glm::ivec2& MapSystem::getOffset() const {
		return mapOffset;
	}

	glm::ivec2 MapSystem::worldToChunk(glm::vec2 pos) const {
		// Chunk position relative to current offset
		const glm::ivec2 chunkOffset = glm::floor(pos / MapChunk::tileSize / glm::vec2{MapChunk::size});

		// Absolute chunk position
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

	void MapSystem::loadChunk(glm::ivec2 pos) {	
		auto& chunk = getChunkAt(pos);

		if (worldToChunk(chunk.getPosition()) != pos) {
			std::cout << "loadChunk: " << "(" << pos.x << ", " << pos.y << ")    " << rand() << "\n";
			loadChunk(chunk, pos);
		}
	}

	void MapSystem::loadChunk(MapChunk& chunk, glm::ivec2 pos) {
		chunk.from(world.getSystem<PhysicsSystem>(), chunkToWorld(pos)); // TODO: Data
	}

	void MapSystem::updateOrigin() {
		// TODO: Move to own system? This should happen before the next frame
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
		}

		if (std::abs(pos.y) > range.y) {
			auto& physSys = world.getSystem<Game::PhysicsSystem>();
			auto dir = std::copysign(1.0f, pos.y);

			physSys.getWorld().ShiftOrigin(b2Vec2{
				0.0f,
				range.y * dir
			});

			mapOffset.y += static_cast<int>(dir);
		}
	}
}
