#pragma once

#if defined(ENGINE_OS_WINDOWS)
	#include <WinSock2.h>
#else
	#error Not yet implemented for this operating system.
#endif

namespace Engine::Net {
	// TODO: doc
	// TODO: split
	class UDPSocket {
		public:
			UDPSocket(uint16 port) : UDPSocket{} {
				// TODO: look into getaddrinfo - https://beej.us/guide/bgnet/html/#getaddrinfoprepare-to-launch
				const sockaddr_in address {
					.sin_family = AF_INET,
					.sin_port = htons(port),
					.sin_addr = INADDR_ANY,
				};

				if (bind(handle, reinterpret_cast<const sockaddr*>(&address), sizeof(address))) {
					const auto err = WSAGetLastError();
					ENGINE_ERROR(err << " - " << getWindowsErrorMessage(err));
				}
			}

			UDPSocket() {
				// TODO: is it possible to make this work with IPv4 and IPv6. AF_USPEC?
				handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

				if (handle == INVALID_SOCKET) {
					// TODO: handle - WSAGetLastError
					ENGINE_ERROR("UDPSocket Error");
				}

				// Set non-blocking
				if (DWORD mode = 1; ioctlsocket(handle, FIONBIO, &mode)) {
					// TODO: handle - WSAGetLastError
					ENGINE_ERROR("UDPSocket Error");
				}
			};

			UDPSocket(const UDPSocket&) = delete;
			UDPSocket& operator=(const UDPSocket&) = delete;

			~UDPSocket() {
				closesocket(handle);
			};

			void send(uint16 port) {
				const char data[] = "This is a test!";

				constexpr uint32 ip = (127 << 24) | (0 << 16) | (0 << 8) | (1 << 0);

				sockaddr_in address;
				address.sin_family = AF_INET;
				address.sin_port = htons(port);
				address.sin_addr.s_addr = htonl(ip);

				auto sent = sendto(
					handle,
					data,
					sizeof(data),
					0,
					reinterpret_cast<const sockaddr*>(&address),
					sizeof(address)
				);

				// TODO: sent can be less than data. look at SO_MAX_MSG_SIZE https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-sendto
				if (sent != sizeof(data)) {
					// TODO: handle
					ENGINE_ERROR("UDPSocket Error");
				}
			}

			void recv() {
				while (true) {
					char data[1024] = {};

					sockaddr_in from;
					int fromlen = sizeof(from);
					int len = recvfrom(
						handle,
						data,
						sizeof(data),
						0,
						reinterpret_cast<sockaddr*>(&from),
						&fromlen
					);

					if (len <= 0) {
						break;
					}

					const auto ip = ntohl(from.sin_addr.s_addr);
					const auto port = ntohs(from.sin_port);

					std::cout
						<< "\n== got data ==" << "\n"
						<< "ip = "
							<< (ip & (255 << 24)) << "."
							<< (ip & (255 << 16)) << "."
							<< (ip & (255 <<  8)) << "."
							<< (ip & (255 <<  0)) << "\n"
						<< "port = " << port << "\n"
						<< "len = " << len << "\n"
						<< "data = " << data << "\n";
				}
			}

		private:
			SOCKET handle;

			/**
			 * Gets the error string for the given error code.
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
