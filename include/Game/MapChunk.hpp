#pragma once

// STD
#include <cmath>

// GLM
#include <glm/glm.hpp>

// Game
#include <Game/MapTile.hpp>
#include <Game/PhysicsSystem.hpp>

// TODO: Should chunks know their world position? there are a few places we do things like `doSomething(chunkPos, getChunkAt(chunkPos))`
// TODO: convert to use sized types - int32, float32, etc.
// TODO: Change all mentions of "tile" to "block". Its a more common term.
// TODO: Doc
namespace Game {
	class MapChunk {
		public:
			constexpr static MapTile AIR{0, false};
			constexpr static MapTile DIRT{1, true};

		public:
			constexpr static glm::ivec2 size = {32, 32};
			constexpr static auto tileSize = 1.0f/6.0f;

		public:
			using EditMemberFunction = void(MapChunk::*)(int,int);
			MapChunk();
			~MapChunk();

			void setup(World& world, GLuint shader, GLuint texture);
			void from(const glm::vec2 wpos);
			void addTile(int x, int y);
			void removeTile(int x, int y);
			void generate();
			void draw(glm::mat4 mvp) const;
			b2Body& getBody() const;

			int data[size.x][size.y] = {};

		private:
			class Vertex {
				public:
					glm::vec2 position;
			};

			b2Body* body = nullptr; // TODO: Cleanup
			Engine::ECS::Entity ent;
			GLuint shader = 0;
			GLuint texture = 0;
			GLuint vao = 0;
			GLuint vbo = 0;
			GLuint ebo = 0;
			GLsizei elementCount = 0;

			void createBody(PhysicsSystem& physSys);
			void updateVertexData(const std::vector<Vertex>& vboData, const std::vector<GLushort>& eboData);
	};
}
