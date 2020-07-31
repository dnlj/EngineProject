#pragma once

// STD
#include <limits>

// Engine
#include <Engine/Engine.hpp>


namespace Engine::Net {
	/**
	 * Wrapping sequence number.
	 * 
	 * Ordering is undefined for any two sequence numbers that are
	 * exactly 2^(N-1) appart. They are neither greater than nor less than each other.
	 */
	enum class SeqNum2 : uint16 {};
	
	ENGINE_INLINE constexpr auto operator+(SeqNum2 a, SeqNum2 b) noexcept {
		using U = std::underlying_type_t<SeqNum2>;
		return SeqNum2{U{a} + U{b}};
	}

	ENGINE_INLINE constexpr auto operator-(SeqNum2 a, SeqNum2 b) noexcept {
		using U = std::underlying_type_t<SeqNum2>;
		return SeqNum2{U{a} - U{b}};
	}

	template<class I> // TODO: C++20 std::integral
	ENGINE_INLINE auto& operator+=(SeqNum2& a, I n) noexcept {
		return reinterpret_cast<std::underlying_type_t<SeqNum2>&>(a) += n, a;
	}
	
	template<class I> // TODO: C++20 std::integral
	ENGINE_INLINE auto& operator-=(SeqNum2& a, I n) noexcept {
		return reinterpret_cast<std::underlying_type_t<SeqNum2>&>(a) -= n, a;
	}

	ENGINE_INLINE auto& operator++(SeqNum2& a) noexcept {
		return a += 1;
	}

	ENGINE_INLINE auto& operator--(SeqNum2& a) noexcept {
		return a -= 1;
	}

	// TODO: just use free standing functions instead of enum and overloads?
	// TODO: change to use a < b && ... || b < a && ...; This handels the 0x8000 case better https://github.com/networkprotocol/reliable/blob/master/reliable.c#L125
	ENGINE_INLINE constexpr bool operator<=(SeqNum2 a, SeqNum2 b) noexcept {
		using U = std::underlying_type_t<SeqNum2>;
		constexpr U m = std::numeric_limits<U>::max();
		constexpr U h = m / 2;
		return static_cast<U>(b - a) <= h;
	}

	ENGINE_INLINE constexpr bool operator>=(SeqNum2 a, SeqNum2 b) noexcept {
		return b <= a;
	}

	ENGINE_INLINE constexpr bool operator<(SeqNum2 a, SeqNum2 b) noexcept {
		return a != b && a <= b;
	}

	ENGINE_INLINE constexpr bool operator>(SeqNum2 a, SeqNum2 b) noexcept {
		return a != b && a >= b;
	}
	
	static_assert(!(SeqNum2{0} > SeqNum2{32768}));
	static_assert(!(SeqNum2{32768} > SeqNum2{0}));

	static_assert(!(SeqNum2{0} >= SeqNum2{32768}));
	static_assert(!(SeqNum2{32768} >= SeqNum2{0}));

	static_assert(!(SeqNum2{0} < SeqNum2{32768}));
	static_assert(!(SeqNum2{32768} < SeqNum2{0}));

	static_assert(!(SeqNum2{0} <= SeqNum2{32768}));
	static_assert(!(SeqNum2{32768} <= SeqNum2{0}));

	static_assert(!(SeqNum2{0} == SeqNum2{32768}));
	static_assert(!(SeqNum2{32768} == SeqNum2{0}));
}
