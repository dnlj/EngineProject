#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Net/Channel.hpp>
#include <Engine/Net/Common.hpp>


namespace Engine::Net {
	class MessageHeader {
		public:
			MessageType type;
			Channel channel;
			// TODO: we only need bits for size since we limit messages to 512
			// TODO: name? this is really data size (size - sizeof(header))
			uint16 size;
			SequenceNumber sequence; // TODO: look into wrapping sequence numbers
	};
	static_assert(sizeof(MessageHeader) == 8);
}
