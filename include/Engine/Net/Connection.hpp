#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Net/AckData.hpp>
#include <Engine/Net/IPv4Address.hpp>
#include <Engine/Net/MessageHeader.hpp>
#include <Engine/Net/Net.hpp>
#include <Engine/Net/PacketWriter.hpp>
#include <Engine/Net/PacketReader.hpp>
#include <Engine/Clock.hpp>
#include <Engine/StaticVector.hpp>


namespace Engine::Net {
	class Connection {
		public:
			PacketReader reader;
			PacketWriter writer;
			Clock::TimePoint lastMessageTime;

		public:
			Connection(UDPSocket& sock, IPv4Address addr = {}, Clock::TimePoint lastMessageTime = {});
			Connection(const Connection&) = delete;
			Connection(const Connection&&) = delete;

			void writeRecvAcks(Channel ch);

			/**
			 * Gets the most recently associated address.
			 * Set from either #reset or #recv.
			 */
			IPv4Address address() const;
	};
}
