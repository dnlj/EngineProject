// STD
#include <iterator>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// GLM
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Engine
#include <Engine/Utility/Utility.hpp>

// Game
#include <Game/MapChunk.hpp>
#include <Game/SpriteSystem.hpp>


namespace Game {
	MapChunk::MapChunk() {
	}

	MapChunk::~MapChunk() {
	}

	void MapChunk::setup(World& world, GLuint shader, GLuint texture) {
		ent = world.createEntity(true);
		auto& physSys = world.getSystem<PhysicsSystem>();
		createBody(physSys);
	}

	void MapChunk::from(const glm::vec2 wpos, const glm::ivec2 chunkPos) {
		body->SetTransform(b2Vec2{wpos.x, wpos.y}, 0.0f);
		pos = chunkPos;
	}

	void MapChunk::addTile(int x, int y) {
		data[x][y] = DIRT.id;
	}

	void MapChunk::removeTile(int x, int y) {
		data[x][y] = AIR.id;
	}

	void MapChunk::createBody(PhysicsSystem& physSys) {
		b2BodyDef bodyDef;
		bodyDef.type = b2_staticBody;
		bodyDef.awake = false;
		bodyDef.fixedRotation = true;

		body = physSys.createBody(ent, bodyDef);
	}

	// TODO: Can we flatten this lambda mess? Seems a bit much.
	// TODO: can this be split up more? Should be able to
	// TODO: Rename to rebuild or similar? generate is a bad name.
	void MapChunk::generate() {
		// TODO: Look into edge and chain shapes
		// Clear all fixtures
		for (auto* fixture = body->GetFixtureList(); fixture;) {
			auto* next = fixture->GetNext();
			body->DestroyFixture(fixture);
			fixture = next;
		}

		b2PolygonShape shape;
		b2FixtureDef fixtureDef;
		fixtureDef.shape = &shape;

		bool used[size.x][size.y]{};

		const auto expand = [&](const int x0, const int y0){
			int x = x0;
			int y = y0;

			const auto useable = [&](const auto& value) {
				return value && !*(&used[0][0] + (&value - &data[0][0]));
			};

			while (y < size.y && useable(data[x][y])) { ++y; }
			if (y == y0) { return; }

			do {
				std::fill(&used[x][y0], &used[x][y], true);
				++x;
			} while (x < size.x && std::all_of(&data[x][y0], &data[x][y], useable));

			const auto halfW = tileSize * 0.5f * (x - x0);
			const auto halfH = tileSize * 0.5f * (y - y0);
			const auto center = b2Vec2(
				x0 * tileSize + halfW,
				y0 * tileSize + halfH
			);

			shape.SetAsBox(halfW, halfH, center, 0.0f);
			body->CreateFixture(&fixtureDef);
		};

		for (int x = 0; x < size.x; ++x) {
			for (int y = 0; y < size.y; ++y) {
				// TODO: Also try a recursive expand
				expand(x, y);
			}
		}
	}

	b2Body& MapChunk::getBody() const {
		return *body;
	}
	
	glm::ivec2 MapChunk::getPosition() const {
		return pos;
	}
}
