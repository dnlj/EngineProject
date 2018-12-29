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

					FlatQuadtree() {
						memset(data, -1, sizeof(data));
					};

					// TODO: Move
					// TODO: Doc
					constexpr static uint32_t spaceBits(uint32_t a) {
						a = a & 0x0000FFFF;
						a = (a ^ a << 8) & 0x00FF00FF;
						a = (a ^ a << 4) & 0x0F0F0F0F;
						a = (a ^ a << 2) & 0x33333333;
						a = (a ^ a << 1) & 0x55555555;
						return a;
					}

					// TODO: Move
					// TODO: Doc
					constexpr static uint32_t compactBits(uint32_t a) {
						a = a & 0x55555555;
						a = (a ^ a >> 1) & 0x33333333;
						a = (a ^ a >> 2) & 0x0F0F0F0F;
						a = (a ^ a >> 4) & 0x00FF00FF;
						a = (a ^ a >> 8) & 0x0000FFFF;
						return a;
					}

					// TODO: Move
					// TODO: Doc
					constexpr static uint32_t interleaveBits(uint32_t a, uint32_t b) {
						return spaceBits(a) | spaceBits(b) << 1;
					}

					void setv(int x, int y) { // TODO: Better name
						const auto temp = interleaveBits(x, y); // TODO: REmove 
						set(interleaveBits(x, y), 1);
					}

					void unsetv(int x, int y) { // TODO: Better name
						unset(interleaveBits(x, y), 1);
					}

					void set(int current, int currentSize) {
						// TODO: Dont i need to check depth not just empty?
						//if (data[current] != -1) { return; } 

						const auto d = getDepth(currentSize); // TODO: Remove;
						memset(data + current, getDepth(currentSize), currentSize);

						const auto parentSize = currentSize * 4;
						update(getParentIndex(current, parentSize), parentSize);
					}

					void unset(int current, int currentSize) {
						// TODO: Dont i need to check depth not just empty?
						//if (data[current] == -1) { return; }

						memset(data + current, -1, currentSize);

						const auto parentSize = currentSize * 4;
						update(getParentIndex(current, parentSize), parentSize);
					}

					// TODO: could pass around depth so we dont need to calculate it every time
					void update(int current, int currentSize) {
						const auto childSize = currentSize / 4;
						const auto first  = data[getChildIndex(current, 0, childSize)];
						const auto second = data[getChildIndex(current, 1, childSize)];
						const auto third  = data[getChildIndex(current, 2, childSize)];
						const auto fourth = data[getChildIndex(current, 3, childSize)];
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
						return child * childSize;
					}

					int getChildIndex(int current, int child, int childSize) const {
						return current + getChildOffset(child, childSize);
					}

					int getParentIndex(int current, int parentSize) const {
						return (current / parentSize) * parentSize;
					}

					template<class Callable>
					void traverse(Callable&& callable) const {
						traverse(callable, 0, rootSize, 0);
					}

					template<class Callable>
					void traverse(Callable&& func, int current, int currentSize, int depth) const {
						//if (data[current] == -1) { return; } // TODO: Fix

						if (data[current] == depth) {
							func(current, currentSize);
						} else {
							const auto childSize = currentSize / 4;
							if (childSize == 0) { return; }

							const auto nextDepth = depth + 1;

							traverse(func, getChildIndex(current, 0, childSize), childSize, nextDepth);
							traverse(func, getChildIndex(current, 1, childSize), childSize, nextDepth);
							traverse(func, getChildIndex(current, 2, childSize), childSize, nextDepth);
							traverse(func, getChildIndex(current, 3, childSize), childSize, nextDepth);
						}
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
				/*{3, 0, 0, 0, 0, 0, 0, 3},
				{0, 2, 0, 0, 0, 0, 2, 0},
				{0, 0, 2, 2, 2, 2, 0, 0},
				{0, 0, 2, 1, 1, 2, 0, 0},
				{0, 0, 2, 0, 1, 2, 0, 0},
				{0, 0, 2, 2, 2, 2, 0, 0},
				{0, 0, 0, 0, 0, 0, 2, 0},
				{4, 0, 0, 0, 0, 0, 0, 3},*/
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
