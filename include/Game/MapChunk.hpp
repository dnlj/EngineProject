#pragma once

// STD
#include <type_traits>

// GLM
#include <glm/glm.hpp>

// Game
#include <Game/MapBlock.hpp>
#include <Game/systems/PhysicsSystem.hpp>


// TODO: Doc
namespace Game {
	class MapChunk {
		public:
			constexpr static MapBlock AIR{0, false};
			constexpr static MapBlock DIRT{1, true};

		public:
			constexpr static glm::ivec2 size = {32, 32};
			constexpr static auto blockSize = 1.0f/6.0f;

			struct RLEPair {
				BlockId bid;
				uint16 count;
			};
			static_assert(sizeof(RLEPair) == 4); // Ensure tight packing

		public:
			BlockId data[size.x][size.y] = {};
			bool updated = false;

			/*
			// TODO: untested
			void toRLE() const {
				constexpr auto sz = size.x * size.y;
				const BlockId* linear = &data[0][0];

				std::vector<RLEPair> encoding;

				RLEPair pair = {linear[0], 1};
				for (int i = 1; i < sz; ++i) {
					const auto& bid = linear[i];
					if (bid == pair.bid) {
						++pair.count;
					} else {
						// TODO: insert into list
						pair.bid = bid;
						pair.count = 1;
					}
					encoding.push_back(pair);
				}
			}
			
			// TODO: untested
			void fromRLE(RLEPair* begin, const RLEPair* end) {
				constexpr auto sz = size.x * size.y;
				BlockId* linear = &data[0][0];

				RLEPair pair = {linear[0], 1};
				int i = 0;
				while (begin != end) {
					linear[i] = pair.bid;
					++i;
					--pair.count;
					if (pair.count == 0) { ++begin; }
				}
			}*/
	};
}
