#pragma once

// STD
#include <string>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Bitset.hpp>
#include <Engine/Net/IPv4Address.hpp>

namespace Engine::Net {
	// TODo: should we use enum defs for this?
	using MessageType = uint8;
	using SeqNum = uint16;
	using ConnectionState = uint8;

	using AckBitset = Bitset<64, uint64>;

	struct MessageDirection_ {
		enum MessageDirection : uint8 {
			None = 0,
			ServerToClient = 1 << 0,
			ClientToServer = 1 << 1,
			Bidirectional = ServerToClient | ClientToServer,
		};
	};
	using MessageDirection = MessageDirection_::MessageDirection;
	
	class MessageMetaInfo {
		public:
			Engine::Net::MessageDirection dir;
			Engine::Net::ConnectionState sendState;
			Engine::Net::ConnectionState recvState;
			const char* name;
	};

	/**
	 * Specialize to provide info about your messages.
	 * @tparam M The the message to specialize. Should be the same message type used with Connection.
	 * @see MessageTraits2
	 */
	template<auto M>
	const MessageMetaInfo& getMessageMetaInfo() {
		static_assert(M != M, "Unimplemented traits for network message.");
	}
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
