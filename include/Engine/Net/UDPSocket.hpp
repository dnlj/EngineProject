#pragma once

#if ENGINE_OS_WINDOWS
	#include <WinSock2.h>
#else
	#error Not yet implemented for this operating system.
#endif

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Net/IPv4Address.hpp>


namespace Engine::Net {
	// TODO: doc
	class UDPSocket {
		public:
			UDPSocket(const uint16 port);

			UDPSocket(const UDPSocket&) = delete;
			UDPSocket& operator=(const UDPSocket&) = delete;

			~UDPSocket();

			int32 send(const char* data, int32 size, const IPv4Address& address) const;

			int32 recv(char* data, int32 size, IPv4Address& address) const;

		private:
			SOCKET handle;

			/**
			 * Gets the error string (utf-8) for the given error code.
			 * @see https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2
			 */
			std::string getWindowsErrorMessage(int err) const;
	};
}
