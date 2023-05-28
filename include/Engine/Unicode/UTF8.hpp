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

	inline constexpr Point32 to32(const Unit8* begin) {
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
	inline constexpr bool isNewline(const Point32 point) noexcept {
		// TODO: how to handle "CR LF"? really we need to handle this with ptrs like we do with lenth,next,prev,etc.
		// See section "Mandatory breaks" under: https://unicode.org/reports/tr14/#BreakingRules
		// See section "5.8 Newline Guidelines" in the Unicode standard: https://www.unicode.org/versions/Unicode15.0.0/ch05.pdf
		return (+point == 0x000D) // CR, carriage return
			|| (+point == 0x000A) // LF, line feed
			|| (+point == 0x0085) // NEL, next line
			|| (+point == 0x000B) // VT, vertical tab
			|| (+point == 0x000C) // FF, form feed
			|| (+point == 0x2028) // LS, line separator
			|| (+point == 0x2029) // PS, paragraph seperator
			|| 0;
	}

	inline constexpr bool isWhitespace(const Point32 point) noexcept {
		return (+point == 0x0020)
			|| (+point == 0x00A0)
			|| (+point == 0x1680)
			|| (+point == 0x202F)
			|| (+point == 0x205F)
			|| (+point == 0x3000)
			|| (+point >= 0x0009 && +point <= 0x000D)
			|| (+point >= 0x2000 && +point <= 0x200A)
			|| isNewline(point)
			|| 0;
	}
}

namespace Engine::Unicode::UTF8 {
	ENGINE_INLINE constexpr bool isStartOfCodePoint(Unit8 unit) noexcept {
		return !(+unit & 0b1000'0000) || ((+unit & 0b1100'0000) == 0b1100'0000);
	}

	/**
	 * Checks if a code point is a newline.
	 */
	ENGINE_INLINE constexpr bool isNewline(const Unit8* begin) noexcept {
		return UTF32::isNewline(to32(begin));
	}

	ENGINE_INLINE constexpr bool isNewline8(const void* begin) noexcept {
		return isNewline(reinterpret_cast<const Unit8*>(begin));
	}

	/**
	 * Checks if a code point is whitespace.
	 */
	ENGINE_INLINE constexpr bool isWhitespace(const Unit8* begin) noexcept {
		return UTF32::isWhitespace(to32(begin));
	}

	ENGINE_INLINE constexpr bool isWhitespace8(const void* begin) noexcept {
		return isWhitespace(reinterpret_cast<const Unit8*>(begin));
	}

	/**
	 * The length in code points.
	 */
	ENGINE_INLINE constexpr auto length(const Unit8* begin, const Unit8* end) noexcept {
		int64 count = 0;
		auto curr = begin;
		while (curr < end) { count += isStartOfCodePoint(*curr); ++curr; }
		return count;
	}

	/**
	 * @see length
	 */
	template<class T>
	ENGINE_INLINE constexpr auto length(const T& cont) noexcept {
		return length(std::data(cont), std::data(cont) + std::size(cont));
	}

	/**
	 * @see length
	 */
	ENGINE_INLINE constexpr auto length8(const void* begin, const void* end) noexcept {
		return length(reinterpret_cast<const Unit8*>(begin), reinterpret_cast<const Unit8*>(end));
	}

	/**
	 * @see length8
	 */
	template<class T>
	ENGINE_INLINE constexpr auto length8(const T& cont) noexcept {
		return length8(std::data(cont), std::data(cont) + std::size(cont));
	}

	/**
	 * Get the previous code point or @p begin if none found.
	 */
	ENGINE_INLINE inline const Unit8* prev(const Unit8* curr, const Unit8* begin) {
		while (curr > begin && !isStartOfCodePoint(*--curr)) {}
		return curr;
	}

	ENGINE_INLINE inline Unit8* prev(Unit8* curr, const Unit8* begin) {
		return const_cast<Unit8*>(prev(const_cast<const Unit8*>(curr), begin));
	}

	ENGINE_INLINE inline const Unit8* prev8(const void* curr, const void* begin) {
		return prev(reinterpret_cast<const Unit8*>(curr), reinterpret_cast<const Unit8*>(begin));
	}
	
	ENGINE_INLINE inline Unit8* prev8(void* curr, const void* begin) {
		return prev(reinterpret_cast<Unit8*>(curr), reinterpret_cast<const Unit8*>(begin));
	}

	/**
	 * Gets the next code point or @p end if none found.
	 */
	ENGINE_INLINE inline const Unit8* next(const Unit8* curr, const Unit8* end) {
		if (curr >= end) { return end; }
		while (++curr < end && !isStartOfCodePoint(*curr)) {}
		return curr;
	}

	ENGINE_INLINE inline Unit8* next(Unit8* curr, const Unit8* end) {
		return const_cast<Unit8*>(next(static_cast<const Unit8*>(curr), end));
	}

	ENGINE_INLINE inline const Unit8* next8(const void* curr, const void* end) {
		return next(reinterpret_cast<const Unit8*>(curr), reinterpret_cast<const Unit8*>(end));
	}
	
	ENGINE_INLINE inline Unit8* next8(void* curr, const void* end) {
		return next(reinterpret_cast<Unit8*>(curr), reinterpret_cast<const Unit8*>(end));
	}
}
