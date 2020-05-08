#pragma once

// Engine
#include <Engine/Engine.hpp>


namespace Engine::Net {
	class PacketHeader {
		public:
			uint16 protocol = 0b0110'1001'1001'0110;
	};
	static_assert(sizeof(PacketHeader) == 2);
}
