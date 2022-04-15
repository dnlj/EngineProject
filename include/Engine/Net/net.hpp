#pragma once

// STD
#include <string>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Bitset.hpp>
#include <Engine/Net/IPv4Address.hpp>

namespace Engine::Net {
	using MessageType = uint8;
	using SeqNum = uint16;

	using AckBitset = Bitset<64, uint64>;

	/**
	 * Specialize to provide info about your messages.
	 * 
	 * Requires member variables:
	 * - state The state enum for which writting this message is valid. Should be the same enum type passed to Connection.
	 * - name The string name of this message. Used for debugging and logging.
	 * 
	 * @tparam M The the message to specialize. Should be the same message type used with Connection.
	 */
	template<auto M>
	struct MessageTraits {
		static_assert(M != M, "Unimplemented traits for network message.");
	};
}

namespace Engine::Net {
	constexpr static uint16 protocol = 0b0'0110'1001'1001'0110;

	// TODO: Doc
	bool startup();

	// TODO: Doc
	bool shutdown();

	// TODO: Doc
	IPv4Address hostToAddress(const std::string& uri);
}
