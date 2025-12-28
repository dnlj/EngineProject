#pragma once

// Engine
#include <Engine/Engine.hpp>


namespace Engine::Input {
	/**
	 * Known scancodes.
	 * These do not account for different keyboard layouts. Purely hardware positions.
	 * 
	 * @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-rawkeyboard
	 * @see [USB HID to PS/2 Scan Code Translation Table](https://download.microsoft.com/download/1/6/1/161ba512-40e2-4cc9-843a-923143f3456c/translate.pdf)
	 * @see [Keyboard Scan Code Specification](https://download.microsoft.com/download/1/6/1/161ba512-40e2-4cc9-843a-923143f3456c/scancode.doc)
	 * @see https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent/code/code_values
	 * @see https://source.android.com/devices/input/keyboard-devices
	 */
	enum class KeyCode : uint16 {
		None = 0x0000,
		Esc = 0x0001,
		Tab = 0x000F,
		Backtick = 0x0029, // Tilde
		Enter = 0x001C,
		Backspace = 0x000E,
		Backslash = 0x002B,
		Comma = 0x0033,
		Period = 0x0034,
		Slash = 0x0035,
		Minus = 0x000C,
		Equal = 0x000D,
		ScrollLock = 0x0046,

		PrintScreen_0 = 0xE02A, // Sent before and after printscreen when no modifier keys are pressed.
		PrintScreen = 0xE037,
		SysReq = 0x0054, // Alt + PrintScreen

		PauseCtrl = 0xE11D, // Sent before pause
		Pause = 0x0045,
		Break = 0xE046, // Ctrl + Pause

		Insert = 0xE052,
		Home = 0xE047,
		PageUp = 0xE049,
		PageDown = 0xE051,
		Delete = 0xE053,
		End = 0xE04F,

		LWin = 0xE05B,
		RWin = 0xE05C,
		LMeta = 0xE05B,
		RMeta = 0xE05C,

		LShift = 0x002A,
		RShift = 0x0036,

		LAlt = 0x0038,
		RAlt = 0xE038,

		LCtrl = 0x001D,
		RCtrl = 0xE01D,

		Up    = 0xE048,
		Down  = 0xE050,
		Left  = 0xE04B,
		Right = 0xE04D,

		F1  = 0x003B,
		F2  = 0x003C,
		F3  = 0x003D,
		F4  = 0x003E,
		F5  = 0x003F,
		F6  = 0x0040,
		F7  = 0x0041,
		F8  = 0x0042,
		F9  = 0x0043,
		F10 = 0x0044,
		F11 = 0x0057,
		F12 = 0x0058,

		One   = 0x0002,
		Two   = 0x0003,
		Three = 0x0004,
		Four  = 0x0005,
		Five  = 0x0006,
		Six   = 0x0007,
		Seven = 0x0008,
		Eight = 0x0009,
		Nine  = 0x000A,
		Zero  = 0x000B,

		A = 0x001E,
		B = 0x0030,
		C = 0x002E,
		D = 0x0020,
		E = 0x0012,
		F = 0x0021,
		G = 0x0022,
		H = 0x0023,
		I = 0x0017,
		J = 0x0024,
		K = 0x0025,
		L = 0x0026,
		M = 0x0032,
		N = 0x0031,
		O = 0x0018,
		P = 0x0019,
		Q = 0x0010,
		R = 0x0013,
		S = 0x001F,
		T = 0x0014,
		U = 0x0016,
		V = 0x002F,
		W = 0x0011,
		X = 0x002D,
		Y = 0x0015,
		Z = 0x002C,
	};

	ENGINE_INLINE inline auto operator+(KeyCode code) noexcept {
		return static_cast<std::underlying_type_t<KeyCode>>(code);
	}
}
