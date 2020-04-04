#pragma once

// Engine
#include <Engine/Net/IPv4Address.hpp>
#include <Engine/Clock.hpp>


namespace Engine::Net {
	class Connection {
		public:
		Clock::TimePoint lastMessageTime;
	};
}
