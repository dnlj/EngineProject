#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Net/net.hpp>


namespace Engine::Net {
	// TODO: pack
	class MessageHeader {
		public:
			MessageType type;
			// TODO: we only need bits for size since we limit messages to 512
			// TODO: name? this is really data size (size - sizeof(header))
			uint16 size;
			SeqNum seq; // TODO: look into wrapping sequence numbers
	};
	static_assert(sizeof(MessageHeader) == 6);
}
