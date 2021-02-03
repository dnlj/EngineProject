#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Net/Common.hpp>
#include <Engine/Net/MessageHeader.hpp>


namespace Engine::Net {
	constexpr inline int32 ASSUMED_MIN_MTU = 1200; // "Exploring usable Path MTU in the Internet"
	constexpr inline int32 MAX_IP_HEADER_SIZE = 60; // IPv6 = 40, IPv4 w/ options = 60
	constexpr inline int32 UDP_HEADER_SIZE = 8;
	constexpr inline int32 MAX_PACKET_SIZE = ASSUMED_MIN_MTU - MAX_IP_HEADER_SIZE - UDP_HEADER_SIZE;

	class Packet {
		public:
			// 2 bytes protocol
			// 2 bytes seq num // TODO: look into seq num wrapping
			// 2 bytes init ack
			// 2 bytes key
			// 8 bytes ack bitset
			byte head[2 + 2 + 2 + 2 + 8] = {};
			byte body[MAX_PACKET_SIZE - sizeof(head)];

		public:
			// TODO: look into alignment/access rules. May need to memcpy.
			
			auto& getProtocol() { return *reinterpret_cast<uint16*>(&head[0]); }
			auto& getProtocol() const { return *reinterpret_cast<const uint16*>(&head[0]); }
			void setProtocol(uint16 p) { getProtocol() = p; }

			auto& getSeqNum() { return *reinterpret_cast<SeqNum*>(&head[2]); }
			auto& getSeqNum() const { return *reinterpret_cast<const SeqNum*>(&head[2]); }
			void setSeqNum(SeqNum n) { getSeqNum() = n; }

			auto& getNextAck() { return *reinterpret_cast<SeqNum*>(&head[4]); }
			auto& getNextAck() const { return *reinterpret_cast<const SeqNum*>(&head[4]); }
			void setNextAck(SeqNum s) { getNextAck() = s; }

			auto& getKey() { return *reinterpret_cast<SeqNum*>(&head[6]); }
			auto& getKey() const { return *reinterpret_cast<const SeqNum*>(&head[6]); }
			void setKey(uint16 k) { getKey() = k; }

			auto& getAcks() { return *reinterpret_cast<AckBitset*>(&head[8]); }
			auto& getAcks() const { return *reinterpret_cast<const AckBitset*>(&head[8]); }
			void setAcks(const AckBitset& a) { getAcks() = a; }
	};
	static_assert(sizeof(Packet) == MAX_PACKET_SIZE);
}
