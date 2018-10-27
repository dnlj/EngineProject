// Game
#include <Game/MapChunk.hpp>

namespace Game {
	void MapChunk::setup(World& world, glm::vec2 pos) {
		ent = world.createEntity(true);
		generate(world.getSystem<PhysicsSystem>());
		body->SetTransform(b2Vec2{pos.x, pos.y}, 0.0f);
	}

	void MapChunk::addTile(int x, int y, PhysicsSystem& physSys) {
		data[x][y] = DIRT.id;
		generate(physSys);
	}

	void MapChunk::removeTile(int x, int y, PhysicsSystem& physSys) {
		data[x][y] = AIR.id;
		generate(physSys);
	}

	void MapChunk::generate(PhysicsSystem& physSys) {
		// TODO: Look into edge and chain shapes
		b2Vec2 oldPos = b2Vec2_zero;

		if (body != nullptr) {
			oldPos = body->GetPosition();
			physSys.destroyBody(body);
		}

		b2BodyDef bodyDef;
		bodyDef.type = b2_staticBody;
		bodyDef.awake = false;
		bodyDef.fixedRotation = true;

		b2PolygonShape shape;

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &shape;

		body = physSys.createBody(ent, bodyDef);

		bool used[width][height]{};

		auto expand = [&](const int ix, const int iy) {
			int w = 0;
			int h = 0;
			bool expandWidth = true;
			bool expandHeight = true;

			while (expandWidth || expandHeight) {
				if (expandWidth) {
					const auto limit = std::min(iy + h, height);
					for (int y = iy; y < limit; ++y) {
						if (used[ix + w][y] || data[ix + w][y] == AIR.id) {
							if (w == 0) { return; }
							expandWidth = false;
							break;
						}
					}

					if (expandWidth) {
						for (int y = iy; y < limit; ++y) {
							used[ix + w][y] = true;
						}

						++w;

						if (ix + w == width) {
							expandWidth = false;
						}
					}
				}

				if (expandHeight) {
					const auto limit = std::min(ix + w, width);
					for (int x = ix; x < limit; ++x) {
						if (used[x][iy + h] || data[x][iy + h] == AIR.id) {
							if (h == 0) { return; }
							expandHeight = false;
							break;
						}
					}

					if (expandHeight) {
						for (int x = ix; x < limit; ++x) {
							used[x][iy + h] = true;
						}

						++h;

						if (iy + h == height) {
							expandHeight = false;
						}
					}
				}
			}

			shape.SetAsBox(
				tileSize * 0.5f * w,
				tileSize * 0.5f * h,
				b2Vec2(
					(ix + w/2.0f) * tileSize,
					(iy + h/2.0f) * tileSize
				),
				0.0f
			);

			body->CreateFixture(&fixtureDef);
		};

		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				expand(x, y);
			}
		}

		body->SetTransform(oldPos, 0.0f);
	}

	void MapChunk::draw(SpriteSystem& spriteSys) const {
		const auto pos = body->GetPosition();

		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				if (data[x][y] != 0) {
					spriteSys.addSprite({
						2,
						{pos.x + (x + 0.5f) * tileSize, pos.y + (y + 0.5f) * tileSize, 0.0f}
					});
				}
			}
		}
	}
}
