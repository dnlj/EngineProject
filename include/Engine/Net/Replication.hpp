#pragma once

// Engine
#include <Engine/Engine.hpp>


namespace Engine::Net {
	enum class Replication : uint8 {
		NONE = 0,
		ONCE,   // Only sent once at initial network. // TODO: may become useless once UPDATE is impl'd
		ALWAYS, // Unreliable frequent updates
		UPDATE, // Reliable sent when modified
	};
}
