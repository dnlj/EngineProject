#pragma once

#if ENGINE_OS_WINDOWS
	#include <WinSock2.h>
	#include <Ws2tcpip.h>
#else
	#error Not yet implemented for this operating system.
#endif

#define ENGINE_UDP_NETWORK_SIM
#ifdef ENGINE_UDP_NETWORK_SIM
#include <queue>
#include <Engine/Clock.hpp>
#include <Engine/Noise/Noise.hpp>
#include <random>
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

			int32 recv(void* data, int32 size, IPv4Address& address);

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

	#ifdef ENGINE_UDP_NETWORK_SIM
		private:
			// TODO: doc
			struct SimSettings {
				// TODO: better name for hpa? delay? similar?
				Engine::Clock::Duration halfPingAdd = std::chrono::milliseconds{50};
				float32 jitter = 0.75f;
				float32 duplicate = 0.01f;
				float32 loss = 0.25f;
			};
			SimSettings simSettings;

			uint64 seed = 0xAAAA'BBBB'CCCC'DDDD;
			// TODO: PCG
			std::mt19937 mt{std::random_device{}()};

			float32 random() { // TODO: pcg?
				std::uniform_real_distribution<float32> dist{0.0f, std::nextafter(1.0f, 2.0f)};
				return dist(mt);
			}

			struct PacketData {
				Engine::Clock::TimePoint time;
				IPv4Address from;
				std::vector<byte> data;
				bool operator>(const PacketData& other) const { return time > other.time; }
			};

			std::priority_queue<
				PacketData,
				std::vector<PacketData>,
				std::greater<PacketData>
			> packetBuffer;

		public:
			auto& getSimSettings() noexcept { return simSettings; }
	#endif
	};
}

#include <Engine/Net/UDPSocket.ipp>
