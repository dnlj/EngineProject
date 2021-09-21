#if ENGINE_OS_WINDOWS
	#include <WinSock2.h>
	#include <Ws2tcpip.h>
#else
	#error Not yet implemented for this operating system.
#endif


// Engine
#include <Engine/Net/UDPSocket.hpp>

namespace Engine::Net {
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
	}
}


namespace Engine::Net {
	UDPSocket::UDPSocket(const uint16 port) {
		// TODO: is it possible to make this work with IPv4 and IPv6. AF_USPEC?
		handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		if (handle == INVALID_SOCKET) {
			const auto err = WSAGetLastError();
			ENGINE_ERROR(err, " - ", getWindowsErrorMessage(err));
		}

		// Set non-blocking
		if (DWORD mode = 1; ioctlsocket(handle, FIONBIO, &mode)) {
			const auto err = WSAGetLastError();
			ENGINE_ERROR(err, " - ", getWindowsErrorMessage(err));
		}

		// Bind to a port (0 = OS assigned)
		const auto address = IPv4Address{INADDR_ANY, port}.getAs<sockaddr>();
		if (bind(handle, &address, sizeof(address))) {
			const auto err = WSAGetLastError();
			ENGINE_ERROR(err, " - ", getWindowsErrorMessage(err));
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
			if (sent != size) {
				const auto err = WSAGetLastError();
				ENGINE_ERROR(err, " - ", getWindowsErrorMessage(err));
			}
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

	// TODO: use Engine::Windows
	std::string UDPSocket::getWindowsErrorMessage(int err) const {
		WCHAR* wmsg = nullptr;
		FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			err,
			0,
			reinterpret_cast<LPWSTR>(&wmsg),
			0,
			nullptr
		);

		const auto size = WideCharToMultiByte(CP_UTF8, 0, wmsg, -1, nullptr, 0, nullptr, nullptr);
		std::string msg(size, '?');
		WideCharToMultiByte(CP_UTF8, 0, wmsg, -1, msg.data(), size, nullptr, nullptr);
		LocalFree(wmsg);
		return msg;
	}

	void UDPSocket::simPacketSend() {
		while (sendBuffer.size()) {
			const auto& top = sendBuffer.top();
			if (top.time > Engine::Clock::now()) { return; }
			const auto saddr = top.addr.getAs<sockaddr>();
			sendto(handle, reinterpret_cast<const char*>(top.data.data()), static_cast<int>(top.data.size()), 0, &saddr, sizeof(saddr));
			sendBuffer.pop();
		}
	}
}
