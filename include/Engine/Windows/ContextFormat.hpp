#pragma once

// Engine
#include <Engine/Engine.hpp>


namespace Engine::Windows {
	struct ContextFormat {
		int32 majorVersion = 4;
		int32 minorVersion = 5;
		bool debug = false;
	};
};
