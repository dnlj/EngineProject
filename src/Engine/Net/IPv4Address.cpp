// Engine
#include <Engine/Net/IPv4Address.hpp>


namespace Engine::Net {
	IPv4Address::IPv4Address(uint32 address, uint32 port)
		: address{address}
		, port{port} {
	}

	IPv4Address::IPv4Address(uint8 a, uint8 b, uint8 c, uint8 d, uint32 port)
		: a{a}, b{b}, c{c}, d{d}
		, port{port} {
	}

	IPv4Address::IPv4Address(const sockaddr_in& saddress)
		: address{ntohl(saddress.sin_addr.s_addr)}
		, port{ntohs(saddress.sin_port)} {
	}
	
	IPv4Address::IPv4Address(const sockaddr& saddress)
		: IPv4Address{reinterpret_cast<const sockaddr_in&>(saddress)} {
	}

	sockaddr_in IPv4Address::getInternetAddress() const {
		sockaddr_in saddr;
		saddr.sin_family = AF_INET,
		saddr.sin_port = htons(port),
		saddr.sin_addr.s_addr = htonl(address);
		return saddr;
	}

	sockaddr IPv4Address::getSocketAddress() const {
		const auto& tmp = getInternetAddress();
		return reinterpret_cast<const sockaddr &>(tmp);
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
