#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Net/Common.hpp>


namespace Engine::Net {
	constexpr inline int32 MAX_PACKET_SIZE = 512;
	class Packet {
		public:
			// 2 bytes protocol
			// 4 bytes seq num // TODO: look into seq num wrapping
			// 1 bytes reliable info (only needs 1 bit)
			// 4 bytes init ack
			// 8 bytes ack bitset
			byte head[2 + 4 + 1 + 4 + 8] = {};
			byte body[MAX_PACKET_SIZE - sizeof(head)];

		public:
			// TODO: look into alignment/access rules. May need to memcpy.
			
			auto& getProtocol() { return *reinterpret_cast<uint16*>(&head[0]); }
			auto& getProtocol() const { return *reinterpret_cast<const uint16*>(&head[0]); }
			void setProtocol(uint16 p) { getProtocol() = p; }

			auto& getSeqNum() { return *reinterpret_cast<SeqNum*>(&head[2]); }
			auto& getSeqNum() const { return *reinterpret_cast<const SeqNum*>(&head[2]); }
			void setSeqNum(SeqNum n) { getSeqNum() = n; }

			// TODO: remove this field. Unused
			auto& getReliable() { return *reinterpret_cast<bool*>(&head[6]); }
			auto& getReliable() const { return *reinterpret_cast<const bool*>(&head[6]); }
			void setReliable(bool r) { getReliable() = r; }

			auto& getInitAck() { return *reinterpret_cast<SeqNum*>(&head[7]); }
			auto& getInitAck() const { return *reinterpret_cast<const SeqNum*>(&head[7]); }
			void setInitAck(SeqNum s) { getInitAck() = s; }

			auto& getAcks() { return *reinterpret_cast<AckBitset*>(&head[11]); }
			auto& getAcks() const { return *reinterpret_cast<const AckBitset*>(&head[11]); }
			void setAcks(const AckBitset& a) { getAcks() = a; }
	};
	static_assert(sizeof(Packet) == MAX_PACKET_SIZE);
	inline constexpr int32 MAX_MESSAGE_SIZE = sizeof(Packet::body) - sizeof(MessageHeader);
}
