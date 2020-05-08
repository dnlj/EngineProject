#pragma once

// Engine
#include <Engine/Engine.hpp>


namespace Engine::Net {
	using MessageType = uint8;
	using MessageChannel = uint8;
	using SequenceNumber = uint32;

	class MessageHeader {
		public:
			MessageType type;
			MessageChannel channel;
			uint16 _unused;
			SequenceNumber sequence;
	};
	static_assert(sizeof(MessageHeader) == 8);
}
