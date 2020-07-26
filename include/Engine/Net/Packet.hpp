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
			// 1 byte reliable info (only needs 1 bit)
			byte head[7] = {};
			byte body[MAX_PACKET_SIZE - sizeof(head)];

		public:
			auto& getProtocol() { return *reinterpret_cast<uint16*>(&head[0]); }
			auto& getProtocol() const { return *reinterpret_cast<const uint16*>(&head[0]); }
			void setProtocol(uint16 p) { getProtocol() = p; }

			auto& getSeqNum() { return *reinterpret_cast<SeqNum*>(&head[2]); }
			auto& getSeqNum() const { return *reinterpret_cast<const SeqNum*>(&head[2]); }
			void setSeqNum(SeqNum n) { getSeqNum() = n; }

			auto& getReliable() { return *reinterpret_cast<bool*>(&head[6]); }
			auto& getReliable() const { return *reinterpret_cast<const bool*>(&head[6]); }
			void setReliable(bool r) { getReliable() = r; }
	};
	static_assert(sizeof(Packet) == MAX_PACKET_SIZE);
	inline constexpr int32 MAX_MESSAGE_SIZE = sizeof(Packet::body) - sizeof(MessageHeader);
}
