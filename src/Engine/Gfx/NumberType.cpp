// STD
#include <iostream>

// Engine
#include <Engine/Gfx/NumberType.hpp>


namespace Engine::Gfx {
	std::ostream& operator<<(std::ostream& os, NumberType type) {
		os << toString(type);
		return os;
	}
}
