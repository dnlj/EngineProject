// GLM
#include <glm/gtc/matrix_transform.hpp>

// Game
#include <Game/MapSystem.hpp>
#include <Game/PhysicsSystem.hpp>
#include <Game/SpriteSystem.hpp>

namespace Game {
	MapSystem::MapSystem(World& world) : SystemBase{world} {
		// TODO: Before/after
		priorityAfter = world.getSystemID<Game::CameraTrackingSystem>();
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
			auto mpos = camera->screenToWorld(input->getMousePosition());
			constexpr auto pos = glm::vec2{0, 0};
			auto bounds = pos + glm::vec2{chunkCountX * MapChunk::width, chunkCountY * MapChunk::height};

			auto offset = mpos - pos;
			offset /= MapChunk::tileSize;

			// TODO: this only works if pos = 0,0
			if (offset.x < 0 || offset.x >= bounds.x) { return; }
			if (offset.y < 0 || offset.y >= bounds.y) { return; }

			const auto ix = static_cast<int>(offset.x / MapChunk::width);
			const auto iy = static_cast<int>(offset.y / MapChunk::height);

			(chunks[ix][iy].*func)(
				static_cast<int>(offset.x - ix * MapChunk::width),
				static_cast<int>(offset.y - iy * MapChunk::height),
				world.getSystem<PhysicsSystem>()
			);
		};

		if (input->isPressed("edit_place")) {
			applyEdit(&MapChunk::addTile);
		} else if (input->isPressed("edit_remove")) {
			applyEdit(&MapChunk::removeTile);
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
}
