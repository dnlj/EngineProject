#if ENGINE_OS_WINDOWS
	#include <WinSock2.h>
	#include <Ws2tcpip.h>
	#include <Engine/Win32/Win32.hpp>
#else
	#error Not yet implemented for this operating system.
#endif

// Engine
#include <Engine/Net/UDPSocket.hpp>


#ifdef ENGINE_UDP_NETWORK_SIM
#include <queue>
#include <random>
#include <pcg_random.hpp>
#include <Engine/Clock.hpp>
#endif


#ifdef ENGINE_UDP_NETWORK_SIM
namespace Engine::Net {
	class UDPSimState {
		private:
			UDPSimSettings settings;

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

			void simPacket(Queue& buff, const IPv4Address& addr, const void* data, const int32 size) {
				if (random() < settings.loss) { return; }

				// Ping var is total variance so between ping +- pingVar/2
				const float32 r = -1 + 2 * random();
				const auto var = std::chrono::duration_cast<Engine::Clock::Duration>(
					// TODO: this def of jitter is idff than we use in Connection. should be +- jitter not +- 0.5 jitter
					settings.halfPingAdd * (settings.jitter * 0.5f * r)
				);

				const auto now = Engine::Clock::now();
				auto pkt = PacketData{
					.time = now + settings.halfPingAdd + var,
					.addr = addr,
					.data = {static_cast<const byte*>(data), static_cast<const byte*>(data) + size},
				};

				if (random() < settings.duplicate) {
					buff.push(pkt);
				}

				buff.push(std::move(pkt));
			}

		public:
			ENGINE_INLINE auto simSend(const IPv4Address& addr, const void* data, const int32 size) {
				simPacket(sendBuffer, addr, data, size);
				return size;
			}

			ENGINE_INLINE void simRecv(const IPv4Address& addr, const void* data, const int32 size) {
				return simPacket(recvBuffer, addr, data, size);
			}

			void realSend(UDPSocket& socket) {
				while (sendBuffer.size()) {
					const auto& top = sendBuffer.top();
					if (top.time > Engine::Clock::now()) { return; }
					const auto saddr = top.addr.as<sockaddr_in>();
					sendto(
						socket.handle, reinterpret_cast<const char*>(top.data.data()), static_cast<int>(top.data.size()), 0,
						reinterpret_cast<const sockaddr*>(&saddr), sizeof(saddr)
					);
					sendBuffer.pop();
				}
			}

			int32 realRecv(void* data, int32 size, IPv4Address& address) {
				if (!recvBuffer.empty()) {
					const auto& top = recvBuffer.top();
					if (top.time <= Engine::Clock::now()) {
						const auto len = std::min(static_cast<int32>(top.data.size()), size);
						memcpy(data, top.data.data(), len);
						address = top.addr;
						recvBuffer.pop();
						return len;
					}
				}
				return -1;
			}

			auto& getSettings() noexcept { return settings; }
	};
}
#endif

namespace Engine::Net {
	template<>
	bool UDPSocket::setOption<SocketOption::Broadcast, bool>(const bool& value) {
		return 0 == setsockopt(handle, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&value), sizeof(value));
	}
	
	template<>
	bool UDPSocket::setOption<SocketOption::MulticastJoin, IPv4Address>(const IPv4Address& groupAddr) {
		const ip_mreq group = {
			.imr_multiaddr = groupAddr.as<sockaddr_in>().sin_addr,
			.imr_interface = 0,
		};
		return 0 == setsockopt(handle, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<const char*>(&group), sizeof(group));
	}
	template<>
	bool UDPSocket::setOption<SocketOption::MulticastLeave, IPv4Address>(const IPv4Address& groupAddr) {
		const ip_mreq group = {
			.imr_multiaddr = groupAddr.as<sockaddr_in>().sin_addr,
			.imr_interface = 0,
		};
		return 0 == setsockopt(handle, IPPROTO_IP, IP_DROP_MEMBERSHIP, reinterpret_cast<const char*>(&group), sizeof(group));
	}
}


namespace Engine::Net {
	UDPSocket::UDPSocket(const uint16 port, const SocketFlag flags) {
		init(flags);
		bind(port);

		#ifdef ENGINE_UDP_NETWORK_SIM
		sim = new UDPSimState();
		#endif
	}

	UDPSocket::~UDPSocket() {
		if (handle != invalid) { closesocket(handle); }
		delete sim;
	};

	UDPSimSettings& UDPSocket::getSimSettings() noexcept {
		return sim->getSettings();
	}

	void UDPSocket::realSimSend() {
		return sim->realSend(*this);
	}
	
	void UDPSocket::init(const SocketFlag flags) {
		ENGINE_DEBUG_ASSERT(handle == invalid, "Only an uninitialized socket can be initialized.");

		// TODO: is it possible to make this work with IPv4 and IPv6. AF_USPEC?
		int type = SOCK_DGRAM;

		#ifndef ENGINE_OS_WINDOWS
		type |= flags & SocketFlags::NonBlocking ? SOCK_NONBLOCK : 0;
		#endif

		handle = ::socket(AF_INET, type, IPPROTO_UDP);
		if (handle == invalid) { showError(); return; }

		// Set non-blocking. Non-Windows is handled in the call to `socket()`
		#ifdef ENGINE_OS_WINDOWS
		if (flags & SocketFlag::NonBlocking) {
			if (DWORD mode = 1; ioctlsocket(handle, FIONBIO, &mode)) {
				showError();
			}
		}
		#endif

		if (flags & SocketFlag::ReuseAddress) {
			if (int val = 1; setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&val), sizeof(val))) {
				showError();
			}
		}
	}

	void UDPSocket::bind(const uint16 port) {
		ENGINE_DEBUG_ASSERT(handle != invalid, "Socket must have been initialized before binding.");
		// Bind to a port (0 = OS assigned)
		const auto address = IPv4Address{INADDR_ANY, port}.as<sockaddr_in>();
		if (::bind(handle, reinterpret_cast<const sockaddr*>(&address), sizeof(address))) {
			showError();
		}
	}

	int32 UDPSocket::send(const void* data, int32 size, const IPv4Address& address) {
		const auto saddr = address.as<sockaddr_storage>();
		
		#ifdef ENGINE_UDP_NETWORK_SIM
			return sim->simSend(saddr, data, size);
		#endif

		const auto sent = sendto(handle,
			reinterpret_cast<const char*>(data), size, 0,
			reinterpret_cast<const sockaddr*>(&saddr), sizeof(saddr)
		);

		#ifdef DEBUG
			if (sent != size) { showError(); }
		#endif

		return sent;
	}

	int32 UDPSocket::recv(void* data, int32 size, IPv4Address& address) {
		sockaddr_storage from;
		int fromlen = sizeof(from);
		int32 len = recvfrom(handle, static_cast<char*>(data), size, 0, reinterpret_cast<sockaddr*>(&from), &fromlen);
		address = from;

		#ifdef ENGINE_UDP_NETWORK_SIM
		{
			if (len > -1) {
				sim->simRecv(from, data, len);
			}

			return sim->realRecv(data, size, address);
		}
		#endif

		return len;
	}

	IPv4Address UDPSocket::getAddress() const {
		sockaddr_storage addr = {};
		int len = sizeof(addr);
		getsockname(handle, reinterpret_cast<sockaddr*>(&addr), &len);
		return addr;
	}

	void UDPSocket::showError() {
		#ifdef ENGINE_OS_WINDOWS 
		const auto err = WSAGetLastError();
		ENGINE_ERROR(err, " - ", Win32::getLastErrorMessage());
		#else
		#error TODO: impl non windows - check errno
		#endif
	}
}
