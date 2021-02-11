#pragma once

// Engine
#include <Engine/Net/IPv4Address.hpp>


namespace Engine::Net {
	constexpr IPv4Address::IPv4Address(uint32 address, uint32 port)
		: address{address}
		, port{port} {
	}

	constexpr IPv4Address::IPv4Address(uint8 a, uint8 b, uint8 c, uint8 d, uint32 port)
		: d{d}, c{c}, b{b}, a{a}
		, port{port} {
	}
}
