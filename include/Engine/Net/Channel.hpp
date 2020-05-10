#pragma once

// Engine
#include <Engine/Engine.hpp>


namespace Engine::Net {
	enum class Channel : uint8 {
		UNRELIABLE,
		RELIABLE,
		ORDERED,
		_COUNT,
	};
}
