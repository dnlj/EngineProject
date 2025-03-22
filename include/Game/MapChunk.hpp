#pragma once

// STD
#include <type_traits>

// GLM
#include <glm/glm.hpp>

// Game
#include <Game/BlockMeta.hpp>
#include <Game/systems/PhysicsSystem.hpp>


// TODO: Doc
namespace Game {
	class MapChunk {
		public:
			constexpr static BlockId RLE_COUNT_BIT = static_cast<BlockId>(1ll << ((sizeof(BlockId) * 8) - 1));

			struct RLEPair {
				BlockId bid;
				uint16 count;
			};
			static_assert(sizeof(RLEPair) == 4); // Ensure tight packing

		public:
			BlockId data[chunkSize.x][chunkSize.y] = {};

		public:
			bool apply(const MapChunk& edit) {
				bool editMade = false;

				for (int x = 0; x < chunkSize.x; ++x) {
					for (int y = 0; y < chunkSize.y; ++y) {
						const auto& ed = edit.data[x][y];
						if (ed == BlockId::None) { continue; }

						auto& cd = data[x][y];
						if (cd == ed) { continue; }

						editMade = true;
						cd = ed;
					}
				}

				return editMade;
			}

			// TODO: move RLE data to active chunk data instead on MapChunk. we only need it if it is active.
			void toRLE(std::vector<byte>& encoding) const {
				encoding.clear();

				// Reserve space for position data
				encoding.insert(encoding.end(), sizeof(RealmId) + sizeof(ChunkVec::x) + sizeof(ChunkVec::y), 0);

				constexpr auto sz = chunkSize.x * chunkSize.y;
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

				// Debug compression ratio
				//{
				//	const uint64 original = sizeof(data);
				//	const uint64 compressed = encoding.size();
				//	const auto ratio = std::imaxdiv(original, compressed);
				//	ENGINE_LOG2("Compression Ratio: {:4}.{}", ratio.quot, ratio.rem);
				//}
			}
			
			bool fromRLE(const byte* begin, const byte* end) {
				bool editMade = false;
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
						if (pair.bid != BlockId::None) {
							editMade = editMade || (linear[i] == pair.bid);
							linear[i] = pair.bid;
						}

						++i;
						--pair.count;
					}
				}

				return editMade;
			}
	};
}
