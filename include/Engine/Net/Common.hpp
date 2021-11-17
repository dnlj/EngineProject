#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Bitset.hpp>


namespace Engine::Net {
	using MessageType = uint8;
	using SeqNum = uint16;

	using AckBitset = Bitset<64, uint64>;

	
	template<auto M>
	struct MessageTraits {
		static_assert(M != M, "Unimplemented type traits for network message.");
		// TODO: doc required members and types
	};
}
