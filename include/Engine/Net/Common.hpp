#pragma once

// Engine
#include <Engine/Engine.hpp>

namespace Engine::Net {
	using MessageType = uint8;
	using SeqNum = uint32; // TODO: look into warpped seq nums

	// TODO: rm - unused
	inline constexpr int32 MAX_UNACKED_MESSAGES = 64;
}
