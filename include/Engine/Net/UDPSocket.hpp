#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Net/IPv4Address.hpp>
#include <Engine/Net/SocketOption.hpp>
#include <Engine/Net/SocketFlag.hpp>

#if ENGINE_CLIENT
#define ENGINE_UDP_NETWORK_SIM
#endif


namespace Engine::Net {
	#ifdef ENGINE_UDP_NETWORK_SIM
		class UDPSimSettings {
			public:
				// TODO: really it may be better to use ticks instead of ms. Since it wont be re-checked until next tick the min time is 1/tickrate.
				Engine::Clock::Duration halfPingAdd = std::chrono::milliseconds{0};
				float32 jitter = 0.0f;
				float32 duplicate = 0.0f;
				float32 loss = 0.0f;
		};

		class UDPSimState;
	#endif

	class UDPSocket {
		#ifdef ENGINE_UDP_NETWORK_SIM
		private:
			friend class UDPSimState;
			UDPSimState* sim = nullptr;

		public:
			UDPSimSettings& getSimSettings() noexcept;
			void realSimSend();
		#endif


		private:
			constexpr static uint64 invalid = -1;
			uint64 handle = invalid;
			void showError();

		public:
			struct DoNotInitialize {
				constexpr explicit DoNotInitialize() = default;
			} constexpr static doNotInitialize;

		public:
			ENGINE_INLINE UDPSocket(DoNotInitialize) noexcept {};

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
				
				#ifdef ENGINE_UDP_NETWORK_SIM
					swap(a.sim, b.sim);
				#endif
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
				static_assert(ENGINE_TMP_FALSE(Value), "Invalid SocketOption + Value combination.");
				return false;
			}

			template<> bool setOption<SocketOption::Broadcast, bool>(const bool& value);
			template<> bool setOption<SocketOption::MulticastJoin, IPv4Address>(const IPv4Address& groupAddr);
			template<> bool setOption<SocketOption::MulticastLeave, IPv4Address>(const IPv4Address& groupAddr);
	};
}
