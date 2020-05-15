// Engine
#include <Engine/Net/UDPSocket.hpp>


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
		const auto address = IPv4Address{INADDR_ANY, port}.getSocketAddress();
		if (bind(handle, &address, sizeof(address))) {
			const auto err = WSAGetLastError();
			ENGINE_ERROR(err, " - ", getWindowsErrorMessage(err));
		}
	}

	UDPSocket::~UDPSocket() {
		closesocket(handle);
	};

	int32 UDPSocket::send(const char* data, int32 size, const IPv4Address& address) const {
		const auto saddr = address.getSocketAddress();
		const auto sent = sendto(handle, data, size, 0, &saddr, sizeof(saddr));

		#ifdef DEBUG
			if (sent != size) {
				const auto err = WSAGetLastError();
				ENGINE_ERROR(err, " - ", getWindowsErrorMessage(err));
			}
		#endif

		return sent;
	}

	int32 UDPSocket::recv(void* data, int32 size, IPv4Address& address) const {
		sockaddr_in from;
		int fromlen = sizeof(from);
		int32 len = recvfrom(handle, static_cast<char*>(data), size, 0, reinterpret_cast<sockaddr*>(&from), &fromlen);
		address = from;
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
}
