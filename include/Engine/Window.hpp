#pragma once

// Engine
#include <Engine/Win32/OpenGLWindow.hpp>


namespace Engine {
	#ifdef ENGINE_OS_WINDOWS
		using Window = Engine::Win32::OpenGLWindow;
	#else
		#error TODO: Impl window for non-win32
	#endif
}
