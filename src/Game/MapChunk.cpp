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
		{ // Vertex array
			glCreateVertexArrays(1, &vao);
		}
	}

	MapChunk::~MapChunk() {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &ebo);
	}

	void MapChunk::setup(World& world, GLuint shader, GLuint texture) {
		this->shader = shader;
		this->texture = texture;
		ent = world.createEntity(true);
		auto& physSys = world.getSystem<PhysicsSystem>();
		createBody(physSys);
	}

	void MapChunk::from(PhysicsSystem& physSys, glm::vec2 pos) {
		body->SetTransform(b2Vec2{pos.x, pos.y}, 0.0f);
		updated = true;
	}

	void MapChunk::addTile(int x, int y, PhysicsSystem& physSys) {
		data[x][y] = DIRT.id;
		collisionTree.setv(x, y);
		updated = true;
	}

	void MapChunk::removeTile(int x, int y, PhysicsSystem& physSys) {
		data[x][y] = AIR.id;
		collisionTree.unsetv(x, y);
		updated = true;
	}

	void MapChunk::createBody(PhysicsSystem& physSys) {
		b2BodyDef bodyDef;
		bodyDef.type = b2_staticBody;
		bodyDef.awake = false;
		bodyDef.fixedRotation = true;

		body = physSys.createBody(ent, bodyDef);
	}

	// TODO: Rename to rebuild or similar? generate is a bad name.
	void MapChunk::generate(PhysicsSystem& physSys) {
		// TODO: Look into edge and chain shapes

		if (!updated) { return; }

		{ // Clear all fixtures
			auto* fixture = body->GetFixtureList();
			while (fixture) {
				auto* next = fixture->GetNext();
				body->DestroyFixture(fixture);
				fixture = next;
			}
		}

		b2PolygonShape shape;
		b2FixtureDef fixtureDef;
		fixtureDef.shape = &shape;

		bool used[size.x][size.y]{};
		std::vector<Vertex> vboData;
		std::vector<GLushort> eboData;
		vboData.reserve(elementCount); // NOTE: This is only an estimate. the correct ratio would be `c * 4/6.0f`
		eboData.reserve(elementCount);

		auto addRect = [&](float halfW, float halfH, b2Vec2 center) {
			const auto size = static_cast<GLushort>(vboData.size());

			eboData.push_back(size + 0);
			eboData.push_back(size + 1);
			eboData.push_back(size + 2);
			eboData.push_back(size + 2);
			eboData.push_back(size + 3);
			eboData.push_back(size + 0);

			vboData.push_back(Vertex{glm::vec2{-halfW + center.x, +halfH + center.y}});
			vboData.push_back(Vertex{glm::vec2{-halfW + center.x, -halfH + center.y}});
			vboData.push_back(Vertex{glm::vec2{+halfW + center.x, -halfH + center.y}});
			vboData.push_back(Vertex{glm::vec2{+halfW + center.x, +halfH + center.y}});

			shape.SetAsBox(halfW, halfH, center, 0.0f);
			body->CreateFixture(&fixtureDef);
		};

		
		auto expand = [&](const int ix, const int iy) {
			int w = 0;
			int h = 0;
			bool expandWidth = true;
			bool expandHeight = true;

			while (expandWidth || expandHeight) {
				if (expandWidth) {
					const auto limit = std::min(iy + h, size.y);
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

						if (ix + w == size.x) {
							expandWidth = false;
						}
					}
				}

				if (expandHeight) {
					const auto limit = std::min(ix + w, size.x);
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

						if (iy + h == size.y) {
							expandHeight = false;
						}
					}
				}
			}

			auto halfW = tileSize * 0.5f * w;
			auto halfH = tileSize * 0.5f * h;
			auto center = b2Vec2(
				(ix + w/2.0f) * tileSize,
				(iy + h/2.0f) * tileSize
			);

			addRect(halfW, halfH, center);
		};

		for (int y = 0; y < size.y; ++y) {
			for (int x = 0; x < size.x; ++x) {
				// TODO: Also try a recursive expand
				expand(x, y);
			}
		}

		elementCount = static_cast<GLsizei>(eboData.size());
		updateVertexData(vboData, eboData);
		updated = false;
	}

	glm::vec2 MapChunk::getPosition() const {
		const auto& pos = body->GetPosition();
		return glm::vec2{pos.x, pos.y};
	}

	void MapChunk::draw(glm::mat4 mvp) const {
		if (elementCount == 0) { return; }

		auto& pos = body->GetPosition();
		mvp = glm::translate(mvp, glm::vec3(pos.x, pos.y, 0.0f));

		glBindVertexArray(vao);
		glUseProgram(shader);

		// Set texture
		glBindTextureUnit(0, texture);
		glUniform1i(5, 0);

		// Set MVP
		glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(mvp));

		// Draw
		glDrawElements(GL_TRIANGLES, elementCount, GL_UNSIGNED_SHORT, 0);
	}

	void MapChunk::updateVertexData(const std::vector<Vertex>& vboData, const std::vector<GLushort>& eboData) {
		constexpr GLuint dataBindingIndex = 0;

		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &ebo);

		{ // Element buffer
			glCreateBuffers(1, &ebo);
			glNamedBufferData(ebo, sizeof(GLushort) * eboData.size(), eboData.data(), GL_STATIC_DRAW);
			glVertexArrayElementBuffer(vao, ebo);
		}

		{ // Vertex buffer
			glCreateBuffers(1, &vbo);
			glNamedBufferData(vbo, sizeof(Vertex) * vboData.size(), vboData.data(), GL_STATIC_DRAW);
			glVertexArrayVertexBuffer(vao, dataBindingIndex, vbo, 0, sizeof(Vertex));
		}

		{ // Vertex attributes
			glEnableVertexArrayAttrib(vao, 0);
			glVertexArrayAttribFormat(vao, 0, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
			glVertexArrayAttribBinding(vao, 0, dataBindingIndex);
		}
	}
}
