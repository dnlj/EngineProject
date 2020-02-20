#pragma once

#if ENGINE_OS_WINDOWS
	#include <WinSock2.h>
#else
	#error Not yet implemented for this operating system.
#endif

// STD
#include <iostream>

// Engine
#include <Engine/Engine.hpp>


namespace Engine::Net {
	class IPv4Address {
		public:
			IPv4Address(uint32 address, uint16 port);
			IPv4Address(uint8 a, uint8 b, uint8 c, uint8 d, uint16 port);
			IPv4Address(const sockaddr_in& saddress);
			IPv4Address(const sockaddr& saddress);

			sockaddr_in getInternetAddress() const;
			sockaddr getSocketAddress() const;

			union {
				uint32 address = 0;
				uint8 parts[4];
				struct {
					uint8 d;
					uint8 c;
					uint8 b;
					uint8 a;
				};
			};

			uint16 port;
	};

	std::ostream& operator<<(std::ostream& os, const IPv4Address& address);
}
