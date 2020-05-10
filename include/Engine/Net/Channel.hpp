#pragma once

// Engine
#include <Engine/Engine.hpp>


namespace Engine::Net {
	enum class Channel : uint8 {
		RELIABLE,
		ORDERED,
		UNRELIABLE,
		_COUNT,
	};
}
