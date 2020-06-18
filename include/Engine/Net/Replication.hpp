#pragma once

// Engine
#include <Engine/Engine.hpp>


namespace Engine::Net {
	enum class Replication : uint8 {
		NONE = 0,
		ALWAYS, // Unreliable frequent updates
		UPDATE, // Reliable sent when modified
	};
}
