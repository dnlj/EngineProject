#pragma once

// Engine
#include <Engine/Net/UDPSocket.hpp>


namespace Engine::Net {
	template<SocketOption Opt, class Value>
	bool UDPSocket::setOption(const Value& value) {
		static_assert(false, "Invalid SocketOption + Value combination.");
		return false;
	}
}
