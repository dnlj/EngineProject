#pragma once


namespace Game {
	enum BlockId : int16 {
		#define X(Name, Solid) Name,
		#include <Game/Blocks.xpp>
		_COUNT,
	};
	ENGINE_BUILD_ALL_BIN_OPS(BlockId);

	struct BlockMeta {
		const BlockId id = BlockId::None;
		const char* const name = nullptr;
		const bool solid = false;
	};

	namespace Detail {
		constexpr inline BlockMeta getBlockMetaLookupArray[] = {
			#define X(Name, Solid) BlockMeta{.id = BlockId::Name, .name = #Name, .solid = Solid},
			#include <Game/Blocks.xpp>
		};
	}

	ENGINE_INLINE constexpr const BlockMeta& getBlockMeta(BlockId bid) noexcept {
		return Detail::getBlockMetaLookupArray[bid];
	}
}
