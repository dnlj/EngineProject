#pragma once

// Engine
#include <Engine/Net/IPv4Address.hpp>
#include <Engine/Clock.hpp>
#include <Engine/Net/MessageStream.hpp>


namespace Engine::Net {
	class Connection {
		public:
			Connection(
				UDPSocket& sock,
				IPv4Address address = {},
				Clock::TimePoint lastMessageTime = {},
				uint32 sequence = {})
				: address{address}
				, lastMessageTime{lastMessageTime}
				, sequence{sequence}
				, writer{sock} {

				// TODO: why not part of writer constructor?
				writer.reset(address);
			}

			IPv4Address address;
			Clock::TimePoint lastMessageTime;
			uint32 sequence;
			Engine::Net::MessageStream writer;
	};
}
