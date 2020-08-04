#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Net/Common.hpp>


namespace Engine::Net {
	class MessageHeader {
		public:
			MessageType type;
			uint8 channel; // TODO: typedef? ChannelId?
			// TODO: we only need bits for size since we limit messages to 512
			// TODO: name? this is really data size (size - sizeof(header))
			uint16 size;
			SeqNum seq; // TODO: look into wrapping sequence numbers
	};
	static_assert(sizeof(MessageHeader) == 6);
}
