#pragma once

// Engine
#include <Engine/Engine.hpp>


namespace Engine::Net {
	class MessageHeader {
		public:
			uint8 type;
			uint8 flags;
			uint16 _filler;
			uint32 sequence;
	};
	static_assert(sizeof(MessageHeader) == 8);
}
