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
					using DepthType = int8_t; // TODO: why not uint8_t ?

					FlatQuadtree() {
						memset(data, nullValue, sizeof(data));
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
						const auto depth = getDepth(currentSize);
						if (data[current] < depth) { return; }

						memset(data + current, depth, currentSize);

						const auto parentSize = currentSize * 4;
						merge(getParentIndex(current, parentSize), parentSize);
					}

					void unset(int current, int currentSize) {
						const auto depth = data[current];
						split(current, getDepth(currentSize));
						memset(data + current, nullValue, currentSize);
					}

					// TODO: Rename
					// TODO: use depth
					void split(int child, int childDepth) {
						if (childDepth == 0) { return; }

						const auto parentDepth = childDepth - 1;
						const auto parent = getParentIndex(child, getSize(parentDepth));
						const auto children = getChildren(parent, getSize(childDepth));


						std::cout << "Parent: " << parent << "-" << getParent(child, parentDepth) << "\n";

						for (const auto c : children) {
							if (data[c] <= parentDepth) {
								memset(data + c, childDepth, getSize(childDepth));
							}
						}

						split(parent, parentDepth);
					}

					// TODO: could pass around depth so we dont need to calculate it every time
					void merge(int current, int currentSize) {
						const auto childSize = currentSize / 4;
						const auto first = data[getChildIndex(current, 0, childSize)];
						const auto second = data[getChildIndex(current, 1, childSize)];
						const auto third = data[getChildIndex(current, 2, childSize)];
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
								merge(getParentIndex(current, parentSize), parentSize);
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

					// TODO: Change to use depth
					int getChildOffset(int child, int childSize) const {
						return child * childSize;
					}

					// TODO: Change to use depth
					int getChildIndex(int current, int child, int childSize) const {
						return current + getChildOffset(child, childSize);
					}

					// TODO: Change to use depth
					std::array<int, 4> getChildren(int current, int childSize) {
						return {
							getChildIndex(current, 0, childSize),
							getChildIndex(current, 1, childSize),
							getChildIndex(current, 2, childSize),
							getChildIndex(current, 3, childSize),
						};
					}

					// TODO: Change to use depth
					int getParentIndex(int current, int parentSize) const {
						return (current / parentSize) * parentSize;
					}

					constexpr static int getParent(int child, DepthType parentDepth) {
						const auto maskBits = 2 * (maxDepth - parentDepth);
						const auto mask = 0xFFFFFFFF << maskBits;
						return child & mask;
					}

					constexpr static int getSize(DepthType depth) {
						return rootSize >> depth * 2;
					}

					template<class Callable>
					void traverse(Callable&& callable) const {
						std::cout << "Max Depth: " << maxDepth << "\n";
						traverse(callable, 0, rootSize, 0);
					}

					template<class Callable>
					void traverse(Callable&& func, int current, int currentSize, DepthType depth) const {
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

					// TODO: Doc
					// TODO: Test
					// TODO: Move
					// TODO: Check that T is integral type
					template<class T>
					constexpr static int count_trailing_zeros(T value) {
						static_assert(std::is_integral<T>::value, "T must be an integral type");

						auto count = std::numeric_limits<T>::digits;

						while (value) {
							value = value << 1;
							--count;
						}

						return count;
					}

				private:
					static_assert(N && !(N & (N - 1)), "Template parameter 'N' must be a power of two.");

					constexpr static int rootSize = N * N;
					constexpr static int maxDepth = count_trailing_zeros(rootSize) / 2 + 1;
					constexpr static DepthType nullValue = std::numeric_limits<DepthType>::max();
					DepthType data[rootSize]{};
			};

		public:
			constexpr static MapTile AIR{0, false};
			constexpr static MapTile DIRT{1, true};

		public:
			constexpr static glm::ivec2 size = {32, 32};
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
