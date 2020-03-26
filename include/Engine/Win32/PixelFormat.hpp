#pragma once

// Engine
#include <Engine/Engine.hpp>


namespace Engine::Win32 {
	struct PixelFormat {
		int32 colorBits = 24;
		int32 alphaBits = 8;
		int32 depthBits = 24;
		int32 stencilBits = 8;
	};
};
