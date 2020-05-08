#pragma once

// Engine
#include <Engine/Net/PacketHeader.hpp>


namespace Engine::Net {
	class Packet {
		public:
			PacketHeader header;
			char data[512 - sizeof(header)];
	};
	static_assert(sizeof(Packet) == 512);
}
