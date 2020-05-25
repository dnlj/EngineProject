#pragma once

// Engine
#include <Engine/Net/Net.hpp>
#include <Engine/Utility/Utility.hpp>


namespace Engine::Net {
	constexpr SequenceNumber seqToIndex(SequenceNumber seq) {
		static_assert(Engine::Utility::isPowerOfTwo(MAX_UNACKED_MESSAGES));
		return seq & (MAX_UNACKED_MESSAGES - 1);
	}
}
