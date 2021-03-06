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

#if ENGINE_CLIENT
#define ENGINE_UDP_NETWORK_SIM
#endif
#ifdef ENGINE_UDP_NETWORK_SIM
#include <queue>
#include <random>
#include <pcg_random.hpp>
#include <Engine/Clock.hpp>
#endif


namespace Engine::Net {
	class UDPSocket {
		public:
			UDPSocket(const uint16 port);
			UDPSocket(const UDPSocket&) = delete;
			UDPSocket& operator=(const UDPSocket&) = delete;

			~UDPSocket();

			int32 send(const void* data, int32 size, const IPv4Address& address);

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
				//Engine::Clock::Duration halfPingAdd = std::chrono::milliseconds{50};
				//float32 jitter = 0.5f;
				//float32 duplicate = 0.01f;
				//float32 loss = 0.01f;

				// TODO: really it may be better to use ticks instead of ms. Since it wont be re-checked until next tick the min time is 1/tickrate.
				Engine::Clock::Duration halfPingAdd = std::chrono::milliseconds{0};
				float32 jitter = 0.0f;
				float32 duplicate = 0.0f;
				float32 loss = 0.0f;
			};
			SimSettings simSettings;

			pcg32 rng = pcg_extras::seed_seq_from<std::random_device>{};

			float32 random() {
				std::uniform_real_distribution<float32> dist{0.0f, std::nextafter(1.0f, 2.0f)};
				return dist(rng);
			}


			struct PacketData {
				Engine::Clock::TimePoint time;
				IPv4Address addr;
				std::vector<byte> data;
				bool operator>(const PacketData& other) const { return time > other.time; }
			};

			using Queue = std::priority_queue<
				PacketData,
				std::vector<PacketData>,
				std::greater<PacketData>
			>;
			Queue sendBuffer;
			Queue recvBuffer;

			void simPacket(Queue& buff, const IPv4Address& addr, const void* data, int32 size) {
				if (random() < simSettings.loss) { return; }

				// Ping var is total variance so between ping +- pingVar/2
				const float32 r = -1 + 2 * random();
				const auto var = std::chrono::duration_cast<Engine::Clock::Duration>(
					// TODO: this def of jitter is idff than we use in Connection. should be +- jitter not +- 0.5 jitter
					simSettings.halfPingAdd * (simSettings.jitter * 0.5f * r)
				);

				const auto now = Engine::Clock::now();
				auto pkt = PacketData{
					.time = now + simSettings.halfPingAdd + var,
					.addr = addr,
					.data = {static_cast<const byte*>(data), static_cast<const byte*>(data) + size},
				};

				if (random() < simSettings.duplicate) {
					buff.push(pkt);
				}

				buff.push(std::move(pkt));
			}

		public:
			void simPacketSend() {
				while (sendBuffer.size()) {
					const auto& top = sendBuffer.top();
					if (top.time > Engine::Clock::now()) { return; }
					const auto saddr = top.addr.getSocketAddress();
					sendto(handle, reinterpret_cast<const char*>(top.data.data()), static_cast<int>(top.data.size()), 0, &saddr, sizeof(saddr));
					sendBuffer.pop();
				}
			}

			auto& getSimSettings() noexcept { return simSettings; }
	#endif
	};
}

#include <Engine/Net/UDPSocket.ipp>
