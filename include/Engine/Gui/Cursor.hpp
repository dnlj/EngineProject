#pragma once


namespace Engine::Gui {
	enum class Cursor {
		#ifdef ENGINE_OS_WINDOWS
			// See https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadcursora
			Invalid = 0,
			Normal = 32512,
			Hand = 32649,
			Text = 32513,
			Question,
			Loading,
			Blocked = 32648,
			Move = 32646, // 
			Resize_TL_BR = 32642, // Top Left, Bottom Right
			Resize_BL_TR = 32643, // Bottom Left, Top Right
			Resize_L_R = 32644, // Left, Right
			Resize_T_B = 32645, // Top, Bottom
		#else
			#error TODO: Implement cursors for non-Windows systems
		#endif
	};
}
