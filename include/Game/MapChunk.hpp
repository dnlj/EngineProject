#pragma once

// STD
#include <cmath>

// GLM
#include <glm/glm.hpp>

// Game
#include <Game/MapTile.hpp>
#include <Game/PhysicsSystem.hpp>

// TODO: Doc
namespace Game {
	class MapChunk {
		private:
			// TODO: Move
			// TODO: Split
			// TODO: Change everything to use root/child/parent/current terms
			template<int N>
			class FlatQuadtree {
				public:
					using DepthType = int8_t;

					void setv(int x, int y) { // TODO: Better name
						set(y * N + x, 1);
					}

					void unsetv(int x, int y) { // TODO: Better name
						unset(y * N + x, 1);
					}

					void set(int current, int currentSize) {
						if (data[current] != -1) { return; }

						memset(data + current, getDepth(currentSize), currentSize);

						const auto parentSize = currentSize * 4;
						update(getParentIndex(current, parentSize), parentSize);
					}

					void unset(int current, int currentSize) {
						if (data[current] == -1) { return; }

						memset(data + current, -1, currentSize);

						const auto parentSize = currentSize * 4;
						update(getParentIndex(current, parentSize), parentSize);
					}

					// TODO: could pass around depth so we dont need to calculate it every time
					void update(int current, int currentSize) {
						const auto first  = data[getChildIndex(current, 0, currentSize)];
						const auto second = data[getChildIndex(current, 1, currentSize)];
						const auto third  = data[getChildIndex(current, 2, currentSize)];
						const auto fourth = data[getChildIndex(current, 3, currentSize)];
						const auto depth = getDepth(currentSize);

						// If all the children are at the same depth and they have not yet been merged
						if (first == second
							&& second == third
							&& third == fourth
							&& fourth == depth + 1) {

							memset(data + current, depth, currentSize);
							const auto parentSize = currentSize * 4;

							if (parentSize <= rootSize) {
								update(getParentIndex(current, parentSize), parentSize);
							}
						}
					}

					DepthType getDepth(int size) const {
						// depth = log4(maxSize / size)
						//       = log2(maxSize / size) / 2
						//
						// Could also use `ctz(maxSize / size) / 2` since we always have a power of two for max/size

						return static_cast<DepthType>(
							log2(static_cast<float>(rootSize / size)) / 2
						);
					}

					int getChildOffset(int child, int childSize) const {
						return child * (childSize / 4);
					}

					int getChildIndex(int current, int child, int childSize) const {
						return current + getChildOffset(child, childSize);
					}

					int getParentIndex(int current, int parentSize) const {
						return (current / parentSize) * parentSize;
					}

				private:
					static_assert(N && !(N & (N - 1)), "Template parameter 'N' must be a power of two.");

					constexpr static int rootSize = N * N;
					DepthType data[rootSize]{};
			};

		public:
			constexpr static MapTile AIR{0, false};
			constexpr static MapTile DIRT{1, true};

		public:
			constexpr static glm::ivec2 size = {16, 16};
			constexpr static auto tileSize = 1.0f/5.0f;

		public:
			MapChunk();
			~MapChunk();

			void setup(World& world, GLuint shader, GLuint texture);
			void from(PhysicsSystem& physSys, glm::vec2 pos);
			void addTile(int x, int y, PhysicsSystem& physSys);
			void removeTile(int x, int y, PhysicsSystem& physSys);
			void generate(PhysicsSystem& physSys);
			glm::vec2 getPosition() const;
			void draw(glm::mat4 mvp) const;

		private:
			class Vertex {
				public:
					glm::vec2 position;
			};

			int data[size.x][size.y] = {
				{3, 0, 0, 0, 0, 0, 0, 3},
				{0, 2, 0, 0, 0, 0, 2, 0},
				{0, 0, 2, 2, 2, 2, 0, 0},
				{0, 0, 2, 1, 1, 2, 0, 0},
				{0, 0, 2, 0, 1, 2, 0, 0},
				{0, 0, 2, 2, 2, 2, 0, 0},
				{0, 0, 0, 0, 0, 0, 2, 0},
				{4, 0, 0, 0, 0, 0, 0, 3},
			};

			static_assert(size.x == size.y, "collisionTree expects an equal width and height.");
			FlatQuadtree<size.x> collisionTree;

			b2Body* body = nullptr; // TODO: Cleanup
			Engine::ECS::Entity ent;
			GLuint shader = 0;
			GLuint texture = 0;
			GLuint vao = 0;
			GLuint vbo = 0;
			GLuint ebo = 0;
			GLsizei elementCount = 0;

			bool updated = false;

			void createBody(PhysicsSystem& physSys);
			void updateVertexData(const std::vector<Vertex>& vboData, const std::vector<GLushort>& eboData);
	};
}
