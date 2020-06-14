#pragma once

// STD
#include <iostream>

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

	ENGINE_INLINE std::ostream& operator<<(std::ostream& os, const Channel& ch) {
		return os << static_cast<int32>(ch);
	}
}
