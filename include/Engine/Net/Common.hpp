#pragma once

// Engine
#include <Engine/Engine.hpp>

namespace Engine::Net {
	using MessageType = uint8;
	using SequenceNumber = uint32;

	inline constexpr int32 MAX_UNACKED_MESSAGES = 64;
}
