#pragma once

// Engine
#include <Engine/Net/IPv4Address.hpp>
#include <Engine/Clock.hpp>


namespace Engine::Net {
	class Connection {
		public:
			Connection(
				IPv4Address address,
				Clock::TimePoint lastMessageTime,
				uint32 sequence)
				: address{address}
				, lastMessageTime{lastMessageTime}
				, sequence{sequence} {
			}

			IPv4Address address;
			Clock::TimePoint lastMessageTime;
			uint32 sequence;
	};
}
