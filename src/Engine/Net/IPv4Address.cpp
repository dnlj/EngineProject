#if ENGINE_OS_WINDOWS
	#include <winsock2.h>
#else
	#error Not yet implemented for this operating system.
#endif

// STD
#include <iostream>

// Engine
#include <Engine/Net/IPv4Address.hpp>


namespace Engine::Net {
	IPv4Address::IPv4Address(const sockaddr_in& addr)
		: address{ntohl(addr.sin_addr.s_addr)}
		, port{ntohs(addr.sin_port)} {
	}
	
	IPv4Address::IPv4Address(const sockaddr& addr)
		: IPv4Address{reinterpret_cast<const sockaddr_in&>(addr)} {
		ENGINE_ASSERT(addr.sa_family == AF_INET, "IPv4Address expects an IPv4 address at the sockaddr location.");
		*this = reinterpret_cast<const sockaddr_in&>(addr);
	}

	template<>
	sockaddr_in IPv4Address::as() const noexcept {
		sockaddr_in saddr;
		saddr.sin_family = AF_INET,
		saddr.sin_port = htons(port),
		saddr.sin_addr.s_addr = htonl(address);
		return saddr;
	}

	bool operator==(const IPv4Address& a, const IPv4Address& b) {
		return a.address == b.address
			&& a.port == b.port;
	}

	std::ostream& operator<<(std::ostream& os, const IPv4Address& address) {
		return os
			<< static_cast<int>(address.a) << "."
			<< static_cast<int>(address.b) << "."
			<< static_cast<int>(address.c) << "."
			<< static_cast<int>(address.d) << ":"
			<< address.port;
	}
}
