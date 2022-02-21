#if ENGINE_OS_WINDOWS
	#include <WinSock2.h>
	#include <Ws2tcpip.h>
	#include <Engine/Win32/Win32.hpp>
#else
	#error Not yet implemented for this operating system.
#endif


// Engine
#include <Engine/Net/UDPSocket.hpp>

namespace Engine::Net {
	template<>
	bool UDPSocket::setOption<SocketOption::BROADCAST, bool>(const bool& value) {
		return 0 == setsockopt(handle, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&value), sizeof(value));
	}
	
	template<>
	bool UDPSocket::setOption<SocketOption::MULTICAST_JOIN, IPv4Address>(const IPv4Address& groupAddr) {
		const ip_mreq group = {
			.imr_multiaddr = groupAddr.getAs<sockaddr_in>().sin_addr,
			.imr_interface = 0,
		};
		return 0 == setsockopt(handle, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<const char*>(&group), sizeof(group));
	}
	template<>
	bool UDPSocket::setOption<SocketOption::MULTICAST_LEAVE, IPv4Address>(const IPv4Address& groupAddr) {
		const ip_mreq group = {
			.imr_multiaddr = groupAddr.getAs<sockaddr_in>().sin_addr,
			.imr_interface = 0,
		};
		return 0 == setsockopt(handle, IPPROTO_IP, IP_DROP_MEMBERSHIP, reinterpret_cast<const char*>(&group), sizeof(group));
	}
}


namespace Engine::Net {
	UDPSocket::UDPSocket(const uint16 port, const SocketFlag flags) {
		// TODO: is it possible to make this work with IPv4 and IPv6. AF_USPEC?
		int type = SOCK_DGRAM;

		#ifndef ENGINE_OS_WINDOWS
		type |= flags & SocketFlags::NonBlocking ? SOCK_NONBLOCK : 0;
		#endif

		handle = socket(AF_INET, type, IPPROTO_UDP);

		if (handle == -1) {
			showError();
		}

		// Set non-blocking. Non-Windows is handled in the call to `socket()`
		#ifdef ENGINE_OS_WINDOWS
		if (flags & SocketFlag::NonBlocking) {
			if (DWORD mode = 1; ioctlsocket(handle, FIONBIO, &mode)) {
				showError();
			}
		}
		#endif

		if (flags & SocketFlag::ReuseAddress) {
			if (int val = 1; setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&val), sizeof(val))) {
				showError();
			}
		}

		// Bind to a port (0 = OS assigned)
		const auto address = IPv4Address{INADDR_ANY, port}.getAs<sockaddr>();
		if (bind(handle, &address, sizeof(address))) {
			showError();
		}
	}

	UDPSocket::~UDPSocket() {
		closesocket(handle);
	};

	int32 UDPSocket::send(const void* data, int32 size, const IPv4Address& address) {
		const auto saddr = address.getAs<sockaddr>();
		
		#ifdef ENGINE_UDP_NETWORK_SIM
		{
			simPacket(sendBuffer, saddr, data, size);
			return size;
		}
		#endif

		const auto sent = sendto(handle, reinterpret_cast<const char*>(data), size, 0, &saddr, sizeof(saddr));

		#ifdef DEBUG
			if (sent != size) { showError(); }
		#endif

		return sent;
	}

	int32 UDPSocket::recv(void* data, int32 size, IPv4Address& address) {
		sockaddr_in from;
		int fromlen = sizeof(from);
		int32 len = recvfrom(handle, static_cast<char*>(data), size, 0, reinterpret_cast<sockaddr*>(&from), &fromlen);
		address = from;

		#ifdef ENGINE_UDP_NETWORK_SIM
		{
			if (len > -1) {
				simPacket(recvBuffer, from, data, len);
			}

			if (recvBuffer.size()) {
				const auto& top = recvBuffer.top();
				if (top.time <= Engine::Clock::now()) {
					memcpy(data, top.data.data(), top.data.size());
					len = static_cast<int32>(top.data.size());
					address = top.addr;
					recvBuffer.pop();
					return len;
				}
			}

			return -1;
		}
		#endif

		return len;
	}

	IPv4Address UDPSocket::getAddress() const {
		sockaddr addr = {};
		int len = sizeof(addr);
		getsockname(handle, &addr, &len);
		return addr;
	}

	void UDPSocket::showError() {
		#ifdef ENGINE_OS_WINDOWS 
		const auto err = WSAGetLastError();
		ENGINE_ERROR(err, " - ", Win32::getLastErrorMessage());
		#else
		#error TODO: impl non windows - check errno
		#endif
	}

#ifdef ENGINE_UDP_NETWORK_SIM
	void UDPSocket::simPacketSend() {
		while (sendBuffer.size()) {
			const auto& top = sendBuffer.top();
			if (top.time > Engine::Clock::now()) { return; }
			const auto saddr = top.addr.getAs<sockaddr>();
			sendto(handle, reinterpret_cast<const char*>(top.data.data()), static_cast<int>(top.data.size()), 0, &saddr, sizeof(saddr));
			sendBuffer.pop();
		}
	}
#endif
}
