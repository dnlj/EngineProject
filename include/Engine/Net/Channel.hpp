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

	ENGINE_INLINE Channel& operator++(Channel& ch) {
		++reinterpret_cast<std::underlying_type_t<Channel>&>(ch);
		return ch;
	}
}
