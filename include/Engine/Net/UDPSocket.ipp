#pragma once

// Engine
#include <Engine/Net/UDPSocket.hpp>


namespace Engine::Net {
	template<SocketOption Opt, class Value>
	bool UDPSocket::setOption(const Value& value) {
		static_assert(false, "Invalid SocketOption + Value combination.");
		return false;
	}

	// TODO: rm
	/*
	template<>
	inline bool UDPSocket::setOption<SocketOption::BROADCAST, bool>(const bool& value) {
		return 0 == setsockopt(handle, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&value), sizeof(value));
	}
	
	template<>
	inline bool UDPSocket::setOption<SocketOption::MULTICAST_JOIN, IPv4Address>(const IPv4Address& groupAddr) {
		const ip_mreq group = {
			.imr_multiaddr = groupAddr.getAs<sockaddr_in>().sin_addr,
			.imr_interface = 0,
		};
		return 0 == setsockopt(handle, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<const char*>(&group), sizeof(group));
	}
	template<>
	inline bool UDPSocket::setOption<SocketOption::MULTICAST_LEAVE, IPv4Address>(const IPv4Address& groupAddr) {
		const ip_mreq group = {
			.imr_multiaddr = groupAddr.getAs<sockaddr_in>().sin_addr,
			.imr_interface = 0,
		};
		return 0 == setsockopt(handle, IPPROTO_IP, IP_DROP_MEMBERSHIP, reinterpret_cast<const char*>(&group), sizeof(group));
	}*/
}
