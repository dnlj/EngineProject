#pragma once

// Engine
#include <Engine/Engine.hpp>

namespace Engine::Unicode::UTF8 {
	ENGINE_INLINE constexpr bool isStartOfCodePoint(byte unit) noexcept {
		return !(unit & 0b1000'0000) || ((unit & 0b1100'0000) == 0b1100'0000);
	}

	
	constexpr bool isWhitespace(const byte* begin, const byte* end) noexcept {
		// TODO: full utf-8 whitespace support
		return *begin == ' ';
	}

	byte* prev(byte* curr, const byte* begin) {
		while (curr > begin && !isStartOfCodePoint(*--curr)) {}
		return curr;
	}

	byte* next(byte* curr, const byte* end) {
		while (++curr < end && !isStartOfCodePoint(*curr)) {}
		return curr;
	}
}
