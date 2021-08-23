#pragma once


namespace Game {
	enum class BlockEntityType {
		#define X(Name) Name,
		#include <Game/BlockEntityType.xpp>
		_count
	};
};
