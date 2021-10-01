#pragma once

// Engine
#include <Engine/Engine.hpp>

// TODO: move
namespace Engine::Unicode::UTF32 {
	/** UTF32 code point */
	enum class Point32 : uint32;
	ENGINE_INLINE inline constexpr auto operator+(const Point32 cp) noexcept { return static_cast<std::underlying_type_t<Point32>>(cp); }

	/** UTF32 code unit */
	enum class Unit32 : uint32;
	ENGINE_INLINE inline constexpr auto operator+(const Unit32 cu) noexcept { return static_cast<std::underlying_type_t<Unit32>>(cu); }
}

namespace Engine::Unicode::UTF8 {
	/** UTF8 code unit */
	enum class Unit8 : uint8;
	ENGINE_INLINE inline constexpr auto operator+(const Unit8 cu) noexcept { return static_cast<std::underlying_type_t<Unit8>>(cu); }
}


// TODO: move
namespace Engine::Unicode {
	using namespace UTF8;
	using namespace UTF32;

	ENGINE_INLINE inline constexpr Point32 to32(const Unit8* begin) {
		uint32 result = {};	
		if (+*begin < 0b1000'0000) {
			result = +*begin;
		} else if (+*begin >= 0b1111'0000) {
			result = +*begin & 0b0000'0111;
			result <<= 6;
			result |= +*++begin & 0b00111111;
			result <<= 6;
			result |= +*++begin & 0b00111111;
			result <<= 6;
			result |= +*++begin & 0b00111111;
		} else if (+*begin >= 0b1110'0000) {
			result = +*begin & 0b0000'1111;
			result <<= 6;
			result |= +*++begin & 0b00111111;
			result <<= 6;
			result |= +*++begin & 0b00111111;
		} else {
			result = +*begin & 0b0001'1111;
			result <<= 6;
			result |= +*++begin & 0b00111111;
		}

		return static_cast<Point32>(result);
	}
}


namespace Engine::Unicode::UTF32 {
	inline constexpr bool isWhitespace(const Point32 point) noexcept {
		// TODO: full utf-8 whitespace support
		//https://util.unicode.org/UnicodeJsps/list-unicodeset.jsp?a=[:whitespace:]
		return +point == ' ';
	}
}

namespace Engine::Unicode::UTF8 {
	ENGINE_INLINE constexpr bool isStartOfCodePoint(Unit8 unit) noexcept {
		return !(+unit & 0b1000'0000) || ((+unit & 0b1100'0000) == 0b1100'0000);
	}

	ENGINE_INLINE constexpr bool isWhitespace(const Unit8* begin, const Unit8* end) noexcept {
		return UTF32::isWhitespace(to32(begin));
	}

	ENGINE_INLINE inline Unit8* prev(Unit8* curr, const Unit8* begin) {
		while (curr > begin && !isStartOfCodePoint(*--curr)) {}
		return curr;
	}

	ENGINE_INLINE inline Unit8* next(Unit8* curr, const Unit8* end) {
		while (++curr < end && !isStartOfCodePoint(*curr)) {}
		return curr;
	}
}
