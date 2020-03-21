// Engine
#include <Engine/Net/UDPSocket.hpp>


namespace Engine::Net {
	UDPSocket::UDPSocket(uint16 port) : UDPSocket{} {
		const auto address = IPv4Address{INADDR_ANY, port}.getSocketAddress();
		if (bind(handle, &address, sizeof(address))) {
			const auto err = WSAGetLastError();
			ENGINE_ERROR(err << " - " << getWindowsErrorMessage(err));
		}
	}

	UDPSocket::UDPSocket() {
		// TODO: is it possible to make this work with IPv4 and IPv6. AF_USPEC?
		handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		if (handle == INVALID_SOCKET) {
			const auto err = WSAGetLastError();
			ENGINE_ERROR(err << " - " << getWindowsErrorMessage(err));
		}

		// Set non-blocking
		if (DWORD mode = 1; ioctlsocket(handle, FIONBIO, &mode)) {
			const auto err = WSAGetLastError();
			ENGINE_ERROR(err << " - " << getWindowsErrorMessage(err));
		}
	};

	UDPSocket::~UDPSocket() {
		closesocket(handle);
	};

	void UDPSocket::send(const IPv4Address& address, const char* data, int size) const {
		const auto saddr = address.getSocketAddress();
		const auto sent = sendto(handle, data, size, 0, &saddr, sizeof(saddr));

		// TODO: sent can be less than data. look at SO_MAX_MSG_SIZE https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-sendto
		// TODO: i think the above is only true for TCP. For UDP its either all sent or we get an error.
		if (sent != size) {
			const auto err = WSAGetLastError();
			ENGINE_ERROR(err << " - " << getWindowsErrorMessage(err));
		}
	}

	void UDPSocket::recv() const {
		while (true) {
			char data[1024] = {};

			sockaddr_in from;
			int fromlen = sizeof(from);
			int len = recvfrom(handle, data, sizeof(data), 0, reinterpret_cast<sockaddr*>(&from), &fromlen);

			if (len <= 0) {
				break;
			}

			IPv4Address address{from};

			std::cout
				<< "\n== got data ==" << "\n"
				<< "from = " << address << "\n"
				<< "len = " << len << "\n"
				<< "data = " << data << "\n";
		}
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
