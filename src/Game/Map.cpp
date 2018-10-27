// Game
#include <Game/Map.hpp>

namespace Game {
	void Map::setup(World& world) {
		for (int y = 0; y < chunkCountY; ++y) {
			for (int x = 0; x < chunkCountX; ++x) {
				chunks[x][y].setup(world, glm::vec2{
					x * MapChunk::width * MapChunk::tileSize,
					y * MapChunk::height * MapChunk::tileSize
				});
			}
		}
	}

	void Map::update(Engine::EngineInstance& engine, World& world) {
		auto& im = engine.inputManager;
		const auto& cam = engine.camera;

		const auto applyEdit = [&](auto func){
			auto mpos = cam.screenToWorld(im.getMousePosition());
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

		if (im.isPressed("edit_place")) {
			applyEdit(&MapChunk::addTile);
		} else if (im.isPressed("edit_remove")) {
			applyEdit(&MapChunk::removeTile);
		}

		{ // TODO: Temp. Should probably generate a mesh
			auto& spriteSys = world.getSystem<Game::SpriteSystem>();

			for (int y = 0; y < chunkCountY; ++y) {
				for (int x = 0; x < chunkCountX; ++x) {
					chunks[x][y].draw(spriteSys);
				}
			}
		}
	}
}
