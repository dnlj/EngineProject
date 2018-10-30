#pragma once

// Game
#include <Game/MapTile.hpp>
#include <Game/PhysicsSystem.hpp>


namespace Game {
	class MapChunk {
		public:
			constexpr static MapTile AIR{0};
			constexpr static MapTile DIRT{1};

		public:
			constexpr static int width = 16;
			constexpr static int height = width;
			constexpr static auto tileSize = 1.0f/4.0f;

		public:
			MapChunk();
			void setup(World& world, glm::vec2 pos);
			void addTile(int x, int y, PhysicsSystem& physSys);
			void removeTile(int x, int y, PhysicsSystem& physSys);
			void generate(PhysicsSystem& physSys);
			void draw(const glm::mat4& mvp) const;

		private:
			class Vertex {
				public:
					glm::vec2 position;
					glm::vec2 texCoord;
			};

			int data[width][height] = {
				{3, 0, 0, 0, 0, 0, 0, 3},
				{0, 2, 0, 0, 0, 0, 2, 0},
				{0, 0, 2, 2, 2, 2, 0, 0},
				{0, 0, 2, 1, 1, 2, 0, 0},
				{0, 0, 2, 0, 1, 2, 0, 0},
				{0, 0, 2, 2, 2, 2, 0, 0},
				{0, 0, 0, 0, 0, 0, 2, 0},
				{4, 0, 0, 0, 0, 0, 0, 3},
			};

			b2Body* body = nullptr; // TODO: Cleanup
			Engine::ECS::Entity ent;
			GLuint shader = 0;
			GLuint vao = 0;
			GLuint vbo = 0;
			GLuint ebo = 0;
			GLsizei elementCount = 0;

			void updateVertexData(const std::vector<Vertex>& vboData, const std::vector<GLushort>& eboData);
	};
}
