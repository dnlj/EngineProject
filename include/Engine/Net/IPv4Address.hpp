#pragma once

// STD
#include <iosfwd>

// Engine
#include <Engine/Types.hpp>
#include <Engine/Hash.hpp>


struct sockaddr_in;
struct sockaddr;


namespace Engine::Net {
	class IPv4Address {
		public:
			IPv4Address() = default;

			constexpr IPv4Address(uint32 address, uint32 port = 0)
				: address{address}
				, port{port} {
			}

			constexpr IPv4Address(uint8 a, uint8 b, uint8 c, uint8 d, uint32 port = 0)
				: d{d}, c{c}, b{b}, a{a}
				, port{port} {
			}

			IPv4Address(const sockaddr_in& saddress);
			IPv4Address(const sockaddr& saddress);

			template<class T>
			T getAs() const noexcept;

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

			uint32 port; // TODO: why is this 32bit? arent ports 16?
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
