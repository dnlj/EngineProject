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

		for (int y = 0; y < chunkCountY; ++y) {
			for (int x = 0; x < chunkCountX; ++x) {
				chunks[x][y].setup(
					world,
					glm::vec2{
						x * MapChunk::width * MapChunk::tileSize,
						y * MapChunk::height * MapChunk::tileSize
					},
					shader.get(),
					texture.get()
				);
			}
		}
	}

	void MapSystem::run(float dt) {
		const auto applyEdit = [&](auto func){
			constexpr auto chunkSize = glm::vec2{MapChunk::width, MapChunk::height};
			constexpr auto mapSize = glm::ivec2{chunkCountX, chunkCountY};
			const auto mpos = camera->screenToWorld(input->getMousePosition());

			// Position, in tiles, relative to current offset
			const auto offset = mpos / MapChunk::tileSize;

			// Offset of tile relative to current offset
			const auto offsetTile = glm::floor(offset);

			// Chunk position relative to current offset
			const auto offsetChunk = glm::ivec2{glm::floor(offsetTile /chunkSize)};

			// Absolute chunk position
			const auto absChunk = mapOffset + offsetChunk;

			// Index for this chunk
			const auto indexChunk = (mapSize + (absChunk % mapSize)) % mapSize;

			// Index of this tile in this chunk
			const auto indexTile = glm::ivec2{glm::fract(offsetTile / chunkSize) * chunkSize};

			(chunks[indexChunk.x][indexChunk.y].*func)(indexTile.x, indexTile.y, world.getSystem<PhysicsSystem>());
		};

		if (input->isPressed("edit_place")) {
			applyEdit(&MapChunk::addTile);
		} else if (input->isPressed("edit_remove")) {
			applyEdit(&MapChunk::removeTile);
		}

		{ // TODO: Move to own system? This should happen before the next frame
			const auto& pos = camera->getPosition();

			if (std::abs(pos.x) > originRange) {
				auto& physSys = world.getSystem<Game::PhysicsSystem>();
				auto dir = std::copysign(1.0f, pos.x);

				physSys.getWorld().ShiftOrigin(b2Vec2{
					originRange * dir,
					0.0f
				});

				mapOffset.x += static_cast<int>(dir);
			}

			if (std::abs(pos.y) > originRange) {
				auto& physSys = world.getSystem<Game::PhysicsSystem>();
				auto dir = originRange * std::copysign(1.0f, pos.y);

				physSys.getWorld().ShiftOrigin(b2Vec2{
					0.0f,
					originRange * dir
				});

				mapOffset.y += static_cast<int>(dir);
			}
		}

		{
			glm::mat4 mvp = camera->getProjection() * camera->getView() * glm::scale(glm::mat4{1.0f}, glm::vec3{1.0f/1});
			for (int y = 0; y < chunkCountY; ++y) {
				for (int x = 0; x < chunkCountX; ++x) {
					chunks[x][y].draw(mvp);
				}
			}
		}
	}
	
	const glm::ivec2& MapSystem::getOffset() const {
		return mapOffset;
	}
}
