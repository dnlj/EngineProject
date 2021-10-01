#pragma once

// Engine
#include <Engine/Engine.hpp>


namespace Engine::Input {
	union Value {
		ENGINE_NO_IMPLICIT_CONVERSION(8);

		int64 i64;
		int32 i32;
		float32 f32;
		glm::vec2 f32v2;

		/**
		 * Check if any value is set.
		 */
		ENGINE_INLINE bool any() const noexcept { return i64; }

		ENGINE_INLINE bool operator==(const Value& other) const noexcept { return i64 == other.i64; }
		ENGINE_INLINE bool operator!=(const Value& other) const noexcept { return !(*this == other); }
	};
	static_assert(sizeof(Value) == 8);
}
