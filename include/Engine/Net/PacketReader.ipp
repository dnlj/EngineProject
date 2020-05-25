#pragma once

// Engine
#include <Engine/Net/PacketReader.hpp>


namespace Engine::Net {
	template<class T>
	decltype(auto) PacketReader::read() {
		if constexpr (std::is_same_v<T, char*>) {
			return reinterpret_cast<const char*>(read(strlen(curr) + 1));
		} else {
			return reinterpret_cast<const T*>(read(sizeof(T)));
		}
	}
}
