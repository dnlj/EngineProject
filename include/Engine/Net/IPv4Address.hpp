#pragma once

// STD
#include <iosfwd>

// Engine
#include <Engine/Types.hpp>
#include <Engine/Hash.hpp>


struct sockaddr_in;
struct sockaddr_storage;
struct sockaddr;


namespace Engine::Net {
	class IPv4Address {
		public:
			constexpr IPv4Address() = default;

			constexpr IPv4Address(uint32 address, uint16 port = 0)
				: address{address}
				, port{port} {
			}

			constexpr IPv4Address(uint8 a, uint8 b, uint8 c, uint8 d, uint16 port = 0)
				: d{d}, c{c}, b{b}, a{a}
				, port{port} {
			}

			IPv4Address(const sockaddr_in& addr);
			
			IPv4Address(const sockaddr& addr)
				: IPv4Address{reinterpret_cast<const sockaddr_in&>(addr)} {
			}

			IPv4Address(const sockaddr_storage& addr)
				: IPv4Address{reinterpret_cast<const sockaddr&>(addr)} {
			};

			template<class T>
			T as() const noexcept;

			union {
				uint32 address = 0;
				uint8 parts[4];
				struct {
					uint8 d;
					uint8 c;
					uint8 b;
					uint8 a;
				};
			};

			uint16 port;
	};

	bool operator==(const IPv4Address& a, const IPv4Address& b);
	std::ostream& operator<<(std::ostream& os, const IPv4Address& address);
}

namespace Engine {
	template<>
	struct Hash<Net::IPv4Address> {
		size_t operator()(const Net::IPv4Address& v) const {
			static_assert(sizeof(v) == 8);
			auto seed = hash(reinterpret_cast<const uint64&>(v));
			return seed;
		}
	};
}

// TODO: where to put this so that this header doesnt depend on fmt
template<>
struct fmt::formatter<Engine::Net::IPv4Address> {
	constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
		return ctx.begin();
	}

	template <typename FormatContext>
	auto format(const Engine::Net::IPv4Address& addr, FormatContext& ctx) -> decltype(ctx.out()) {
		fmt::format_to(ctx.out(), "{}.{}.{}.{}:{}", addr.a, addr.b, addr.c, addr.d, addr.port);
		return ctx.out();
	}
};

