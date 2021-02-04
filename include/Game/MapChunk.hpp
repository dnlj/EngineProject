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
			constexpr static BlockId RLE_COUNT_BIT = static_cast<BlockId>(1ll << ((sizeof(BlockId) * 8) - 1));

			struct RLEPair {
				BlockId bid;
				uint16 count;
			};
			static_assert(sizeof(RLEPair) == 4); // Ensure tight packing

		public:
			BlockId data[size.x][size.y] = {};
			Engine::ECS::Tick updated = {};
			Engine::ECS::Tick lastEncoding = {};
			std::vector<byte> encoding;

			void toRLE() {
				if (lastEncoding == updated) { return; }
				lastEncoding = updated;
				encoding.clear();

				// Reserve space for position data
				encoding.insert(encoding.end(), sizeof(size), 0);

				constexpr auto sz = size.x * size.y;
				const BlockId* linear = &data[0][0];

				RLEPair pair = {
					.bid = linear[0],
					.count = 1,
				};

				 auto&& insert = [&]{
					if (pair.count == 1) {
						BlockId bid = pair.bid | RLE_COUNT_BIT;
						const byte* start = reinterpret_cast<const byte*>(&bid);
						const byte* stop = start + sizeof(bid);
						encoding.insert(encoding.cend(), start, stop);
					} else {
						const byte* start = reinterpret_cast<const byte*>(&pair);
						const byte* stop = start + sizeof(pair);
						encoding.insert(encoding.cend(), start, stop);
					}
				};

				for (int i = 1; i < sz; ++i) {
					const auto& bid = linear[i];
					if (bid == pair.bid) {
						++pair.count;
					} else {
						insert();
						pair.bid = bid;
						pair.count = 1;
					}
				}
				insert();
			}
			
			void fromRLE(const byte* begin, const byte* end) {
				constexpr auto sz = size.x * size.y;
				BlockId* linear = &data[0][0];
				RLEPair pair;

				int i = 0;
				while (begin != end) {

					pair.bid = *reinterpret_cast<const decltype(pair.bid)*>(begin);
					begin += sizeof(pair.bid);

					if (pair.bid & RLE_COUNT_BIT) {
						pair.bid ^= RLE_COUNT_BIT;
						pair.count = 1;
					} else {
						pair.count = *reinterpret_cast<const decltype(pair.count)*>(begin);
						begin += sizeof(pair.count);
					}

					while (pair.count) {
						linear[i] = pair.bid;
						++i;
						--pair.count;
					}
				}
			}
	};
}
