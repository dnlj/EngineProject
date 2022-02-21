#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Net/IPv4Address.hpp>
#include <Engine/Net/SocketOption.hpp>
#include <Engine/Net/SocketFlag.hpp>

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
			struct DoNotInitialize {
				constexpr explicit DoNotInitialize() = default;
			} constexpr static doNotInitialize;

		public:
			ENGINE_INLINE UDPSocket(DoNotInitialize) noexcept {}

			UDPSocket(const uint16 port, const SocketFlag flags = {});

			ENGINE_INLINE UDPSocket(UDPSocket&& other) noexcept {
				*this = std::move(other);
			}

			ENGINE_INLINE UDPSocket& operator=(UDPSocket&& other) noexcept {
				swap(*this, other);
				return *this;
			}

			ENGINE_INLINE friend void swap(UDPSocket& a, UDPSocket& b) noexcept {
				using std::swap;
				swap(a.handle, b.handle);
			}

			UDPSocket() = delete;
			UDPSocket(const UDPSocket&) = delete;
			UDPSocket& operator=(const UDPSocket&) = delete;

			~UDPSocket();

			void init(const SocketFlag flags = {});
			void bind(const uint16 port);

			int32 send(const void* data, int32 size, const IPv4Address& address);
			int32 recv(void* data, int32 size, IPv4Address& address);

			IPv4Address getAddress() const;

			template<SocketOption Opt, class Value>
			bool setOption(const Value& value){
				static_assert(false, "Invalid SocketOption + Value combination.");
				return false;
			}

			template<> bool setOption<SocketOption::Broadcast, bool>(const bool& value);
			template<> bool setOption<SocketOption::MulticastJoin, IPv4Address>(const IPv4Address& groupAddr);
			template<> bool setOption<SocketOption::MulticastLeave, IPv4Address>(const IPv4Address& groupAddr);

		private:
			constexpr static uint64 invalid = -1;
			uint64 handle = invalid;
			void showError();

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
			void simPacketSend();

			auto& getSimSettings() noexcept { return simSettings; }
	#endif
	};
}
