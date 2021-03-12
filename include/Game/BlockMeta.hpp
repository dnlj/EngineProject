#pragma once


namespace Game {
	enum BlockId : int16 {
		#define X(Name, Solid, Path) Name,
		#include <Game/Blocks.xpp>
		_COUNT,
	};
	ENGINE_BUILD_ALL_OPS(BlockId);

	struct BlockMeta {
		const BlockId id = BlockId::None;
		const char* const name = nullptr;
		const bool solid = false;
		const char* path = nullptr;
	};

	namespace Detail {
		constexpr inline BlockMeta getBlockMetaLookupArray[] = {
			#define X(Name, Solid, Path) BlockMeta{.id = BlockId::Name, .name = #Name, .solid = Solid, .path = Path},
			#include <Game/Blocks.xpp>
		};
	}

	ENGINE_INLINE constexpr const BlockMeta& getBlockMeta(BlockId bid) noexcept {
		return Detail::getBlockMetaLookupArray[bid];
	}
}
