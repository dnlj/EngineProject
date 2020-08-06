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

			#ifdef ENGINE_UDP_NETWORK_SIM
				constexpr static Engine::Clock::Duration halfPingAdd = std::chrono::milliseconds{50};
				constexpr static float64 jitter = 0.50;
				constexpr static float64 duplicate = 0.05;
				constexpr static float64 loss = 0.10;
				uint64 seed = 0xAAAA'BBBB'CCCC'DDDD;
				std::mt19937 mt{std::random_device{}()}; // TODO: bad seeding. MT is huge. switch to other rng.
				float64 random() { // TODO: pcg?
					//seed = Noise::lcg(seed);
					//return static_cast<float64>(seed) / std::numeric_limits<decltype(seed)>::max();
					std::uniform_real_distribution<float64> dist{0.0, std::nextafter(1.0, 2.0)};
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
			#endif

			/**
			 * Gets the error string (utf-8) for the given error code.
			 * @see https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2
			 */
			std::string getWindowsErrorMessage(int err) const;
	};
}

#include <Engine/Net/UDPSocket.ipp>
