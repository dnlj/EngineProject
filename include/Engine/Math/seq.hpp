#pragma once

/**
 * Various utilities for working with sequence numbers.
 *
 * A sequences numbers are a continuous sequence of whole unsigned numbers that
 * wrap at max/zero. Every sequence number is greater than the previous and less
 * then the next, contrary to normal unsigned wrapping. For example, with a
 * uint8 sequence number we have:
 *   ... < 254 < 255 < 0 < 1 < 2 < ...
 */
namespace Engine::Math::Seq {
	template<class S>
	struct AssertSequenceNumber : std::true_type {
		static_assert(std::is_unsigned_v<S>, "Sequence numbers must be unsigned");
	};

	template<class S>
	constexpr inline bool AssertSequenceNumber_v = AssertSequenceNumber<S>::value;

	template<uint32 N>
	struct AssertSequenceNeighborhood : std::true_type {
		static_assert(N >= 2,
			"Only one division will result in standard whole number comparison. "
			"For the wrapping behavior to make sense there must be at least two divisions."
		);

		// There isn't technically a reason not to allow other numbers aside
		// from rounding, but it is likely a bug.
		static_assert(std::has_single_bit(N), "The number of divisions must be a power of two.");
	};

	template<uint32 N>
	constexpr inline bool AssertSequenceNeighborhood_v = AssertSequenceNeighborhood<N>::value;

	// TODO: Add generic SeqNum<U> class to avoid annoying type promotion and
	//       allow easier auditing. Type promotion from a-b to int has caused bugs in
	//       the past:
	//template<class Count>
	//class SeqNum {
	//	public:
	//		Count count;
	//
	//		constexpr SeqNum() noexcept {
	//			static_assert(sizeof(SeqNum) == sizeof(Count));
	//		}
	//
	//		constexpr explicit SeqNum(Count count) noexcept : count{count} {}
	//		constexpr explicit operator Count() const noexcept { return count; }
	//
	//		constexpr SeqNum& operator++() noexcept { ++count; return *this; }
	//		constexpr SeqNum& operator--() noexcept { --count; return *this; }
	//
	//		constexpr friend bool operator==(SeqNum lhs, SeqNum rhs) noexcept { return lhs.count == rhs.count; }
	//		constexpr friend bool operator!=(SeqNum lhs, SeqNum rhs) noexcept { return lhs.count != rhs.count; }
	//
	//		constexpr friend SeqNum operator-(SeqNum lhs, Count rhs) noexcept { return SeqNum{lhs.count - rhs}; }
	//		constexpr friend SeqNum operator+(SeqNum lhs, Count rhs) noexcept { return SeqNum{lhs.count + rhs}; }
	//
	//		//constexpr bool nearby(SeqNum other) const noexcept { return Math::Seq::nearby<4>(count, other.count); }
	//		//constexpr SeqNum distant() const noexcept { return SeqNum{Math::Seq::distant<4>(count)}; }
	//
	//		// Do not define these. A sequence number needs to take special care when
	//		// comparing. Its better to have these as separate functions so it isn't
	//		// confused for trivial comparision and it is obvious that something nontrivial is happening.
	//		// See: Engine::Math::Seq
	//		constexpr friend bool operator<(SeqNum lhs, SeqNum rhs) noexcept = delete;
	//		constexpr friend bool operator>(SeqNum lhs, SeqNum rhs) noexcept = delete;
	//		constexpr friend bool operator<=(SeqNum lhs, SeqNum rhs) noexcept = delete;
	//		constexpr friend bool operator>=(SeqNum lhs, SeqNum rhs) noexcept = delete;
	//};

	// TODO: Need to thoroughly unit test this such that:
	//       - nearby(a, distant(a)) == false
	//       - nearby(a+1, distant(a)) == false
	//       - nearby(a+2, distant(a)) == false
	// 
	//       This function is useful so we can represent "invalid" values when
	//       the domain is significantly larger than the use-space. For example,
	//       if we wanted to change Tick to be a sequence number. We only ever
	//       care about +-10s of ticks. Which is 2*64*10=1280 which is way
	//       less than 2^32.
	// 
	//template<uint32 N, class S>
	//requires AssertSequenceNeighborhood_v<N> && AssertSequenceNumber_v<S>
	//ENGINE_INLINE constexpr S distant(S a) noexcept {
	//	// TODO: what do we want here? just domain? domain / 2? (N/2)?
	//	constexpr S dist = std::numeric_limits<S>::max() / N;
	//	return a - dist;
	//}

	/**
	 * Determine if two sequence numbers are within one out of N given divisions
	 * over the domain of the sequence number.
	 */
	template<uint32 N, class S>
	requires AssertSequenceNeighborhood_v<N> && AssertSequenceNumber_v<S>
	ENGINE_INLINE constexpr bool nearby(S a, decltype(a) b) noexcept {
		constexpr S dist = std::numeric_limits<S>::max() / N;
		return ((b > a) && (b - a <= dist))
			|| ((b < a) && (a - b >  dist));
	}

	/**
	 * Check if a sequence number is less than another.
	 */
	template<class S>
	ENGINE_INLINE constexpr bool less(S a, decltype(a) b) noexcept {
		return nearby<2>(a, b);
	}
	
	/**
	 * Check if a sequence number is greater than another.
	 */
	template<class S>
	ENGINE_INLINE constexpr bool greater(S a, decltype(a) b) noexcept {
		return nearby<2>(b, a);
	}

	// TODO: We should be able to implement this cheaper than doing `greater(min) && less(max)`
	//template<class S>
	//ENGINE_INLINE constexpr bool betweenInclusive(S a, S min, S max) noexcept {}
}
