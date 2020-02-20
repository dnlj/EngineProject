#pragma once

#if ENGINE_OS_WINDOWS
	#include <WinSock2.h>
#else
	#error Not yet implemented for this operating system.
#endif

// Engine
#include <Engine/Net/IPv4Address.hpp>

namespace Engine::Net {
	// TODO: doc
	// TODO: split
	class UDPSocket {
		public:
			UDPSocket(uint16 port) : UDPSocket{} {
				// TODO: look into getaddrinfo - https://beej.us/guide/bgnet/html/#getaddrinfoprepare-to-launch
				const auto address = IPv4Address{INADDR_ANY, port}.getSocketAddress();
				if (bind(handle, &address, sizeof(address))) {
					const auto err = WSAGetLastError();
					ENGINE_ERROR(err << " - " << getWindowsErrorMessage(err));
				}
			}

			UDPSocket() {
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

			UDPSocket(const UDPSocket&) = delete;
			UDPSocket& operator=(const UDPSocket&) = delete;

			~UDPSocket() {
				closesocket(handle);
			};

			void send(uint16 port) {
				const char data[] = "This is a test!";
				const auto address = IPv4Address{127, 0, 0, 1, port}.getSocketAddress();

				auto sent = sendto(handle, data, sizeof(data), 0, &address, sizeof(address));

				// TODO: sent can be less than data. look at SO_MAX_MSG_SIZE https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-sendto
				if (sent != sizeof(data)) {
					const auto err = WSAGetLastError();
					ENGINE_ERROR(err << " - " << getWindowsErrorMessage(err));
				}
			}

			void recv() {
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

		private:
			SOCKET handle;

			/**
			 * Gets the error string (utf-8) for the given error code.
			 * @see https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2
			 */
			std::string getWindowsErrorMessage(int err) {
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
	};
}
