#pragma once

#if ENGINE_OS_WINDOWS
	#include <WinSock2.h>
	#include <Ws2tcpip.h>
#else
	#error Not yet implemented for this operating system.
#endif

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Net/IPv4Address.hpp>
#include <Engine/Net/SocketOptions.hpp>


namespace Engine::Net {

	class UDPSocket {
		public:
			UDPSocket(const uint16 port);
			UDPSocket(const UDPSocket&) = delete;
			UDPSocket& operator=(const UDPSocket&) = delete;

			~UDPSocket();

			int32 send(const void* data, int32 size, const IPv4Address& address) const;

			int32 recv(void* data, int32 size, IPv4Address& address) const;

			IPv4Address getAddress() const;

			template<SocketOption Opt, class Value>
			bool setOption(const Value& value);

		private:
			SOCKET handle;

			/**
			 * Gets the error string (utf-8) for the given error code.
			 * @see https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2
			 */
			std::string getWindowsErrorMessage(int err) const;
	};
}

#include <Engine/Net/UDPSocket.ipp>
