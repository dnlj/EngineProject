#pragma once

// Engine
#include <Engine/Net/PacketWriter.hpp>


namespace Engine::Net {
	template<class T>
	void PacketWriter::write(const T& t) {
		write(&t, sizeof(T));
	};

	constexpr int32 PacketWriter::capacity() {
		return sizeof(packet.data);
	}
}
