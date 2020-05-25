#pragma once

// STD
#include <vector>

// Engine
#include <Engine/Net/Common.hpp>


namespace Engine::Net {
	// TODO: doc
	struct AckData {
		SequenceNumber nextAck = 0;
		uint64 acks = 0;
		// TODO: some kind of memory pool and views instead? this seems dumb
		std::vector<char> messages[MAX_UNACKED_MESSAGES] = {};
	};
}
