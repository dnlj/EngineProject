#pragma once


namespace Engine::Gfx {
	enum class NumberType {
		#define X(Name, GLEnum) Name,
		#include <Engine/Gfx/NumberType.xpp>
		// TODO: half float
		// TODO: fixed
		// TODO: special values such as GL_INT_2_10_10_10_REV, GL_UNSIGNED_INT_10F_11F_11F_REV, etc
		_count,
		_int_last = UInt32,
		_float_last = Float64,
	};

	ENGINE_BUILD_DECAY_ENUM(NumberType);

	ENGINE_INLINE constexpr inline bool isInteger(NumberType t) {
		return t < NumberType::_int_last;
	}

	constexpr inline GLenum toGLEnum(NumberType type) {
		constexpr GLenum lookup[+NumberType::_count] = {
			#define X(Name, GLEnum) GLEnum,
			#include <Engine/Gfx/NumberType.xpp>
		};
		return lookup[+type];
	}
}
