#pragma once

// Engine
#include <Engine/Engine.hpp>


namespace Engine::Win32 {
	struct ContextFormat {
		int32 majorVersion = 4;
		int32 minorVersion = 5;
		bool debug = false;
	};
};
