#pragma once

// Engine
#include <Engine/Engine.hpp>


namespace Engine::Input {
	union Value {
		int32 value = 0;
		float32 valuef;

		operator bool() const { return value; }
		operator int32() const { return value; }
		operator float32() const { return valuef; }
	};
	static_assert(sizeof(Value) == sizeof(int32));
}
