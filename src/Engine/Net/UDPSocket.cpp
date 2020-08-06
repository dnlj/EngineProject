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

	int32 UDPSocket::send(const void* data, int32 size, const IPv4Address& address) const {
		const auto saddr = address.getSocketAddress();
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
			// TODO: imgui sliders?
			const auto now = Engine::Clock::now();

			if (len > -1) {
				// Ping var is total variance so between ping +- pingVar/2
				const float64 r = -1 + 2 * random();
				const auto var = std::chrono::duration_cast<Engine::Clock::Duration>(halfPingAdd * (jitter * 0.5 * r));

				if (random() < loss) {
					return -1;
				}

				auto pkt = PacketData{
					.time = now + halfPingAdd + var,
					.from = from,
					.data = {reinterpret_cast<byte*>(data), reinterpret_cast<byte*>(data) + len},
				};

				if (random() < duplicate) {
					packetBuffer.push(pkt);
				}

				packetBuffer.push(std::move(pkt));
			}

			if (packetBuffer.size()) {
				const auto& top = packetBuffer.top();
				if (top.time < now) {
					memcpy(data, top.data.data(), top.data.size());
					len = static_cast<int32>(top.data.size());
					address = top.from;
					packetBuffer.pop();
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
}
