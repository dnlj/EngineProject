#pragma once
// Windows
#include <hidsdi.h> // Needed for type definitions in hidpi.h
#include <hidpi.h>

// GL
#include <glloadgen/gl_core_4_5.hpp>
#include <wglext.h> // TODO: should be part of glloadgen or similar.

// GLM
#include <glm/vec2.hpp>

// Engine
#include <Engine/Win32/PixelFormat.hpp>
#include <Engine/Win32/ContextFormat.hpp>
#include <Engine/Input/InputEvent.hpp>
#include <Engine/WindowCallbacks.hpp>


namespace Engine::Win32 {
	// TODO: GLFW_OPENGL_DEBUG_CONTEXT
	// TODO: GLFW_SRGB_CAPABLE
	// TODO: name?
	// TODO: High DPI https://docs.microsoft.com/en-us/windows/win32/hidpi/high-dpi-desktop-application-development-on-windows
	// TODO: mouse buttons other than l/r
	class OpenGLWindow {
		private:
			struct WGLPointers {
				PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
				PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
				PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
				PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT;
			};

			WGLPointers wglPtrs;

			struct KeyboardState {
				// We use 0xAA as a custom prefix to denote that the stored code is actually a virtual key (Win32 VK_* enum).
				// This is needed because some keys don't produce a scancode (RAWKEYBOARD::MakeCode)
				// but do produce a virtual key (RAWKEYBOARD::VKey). One example of this is WASD Code V2B media keys.
				// We could map the virtual key back to a scancode using MapVirtualKeyW
				// but not all virtual keys have corresponding scancodes so instead we store the VK_* value with
				// the prefix 0xAA = 0b10101010 which is mostly arbitrary value.

				// Must be stored so we can detect repeat (held) keys
				bool state[0xFF * 4]; // For each prefix: 0x00, 0xE0, 0xE1, 0xAA (custom)
			};

			//
			// Using raw input for controllers has some issue.
			//
			// The first is that GetRawInputDeviceInfo (and HidP_GetPreparsedData?)
			// merge the trigger axes (one goes negative, one goes positive, when pressed at the same time the result is no
			// change in that axis). As far as i can tell this is an entirely artificial merging because if you handle
			// reading from the hid device directly, or use a different api, the data is separate.
			//
			// To solve this your options are:
			// 1. Use DirectInput - No
			//    * Deprecated
			//    * Provides nothing over RAWINPUT
			// 2. Use XInput - Supersedes DirectInput. Has separate drive in device manager that handles triggers correctly (also used by WinRT?).
			//    * Has a 4 controller limit
			//    * Doesnt fully support xbox one controllers ("impulse triggers")
			// 3. Use WinRT (Windows.Gaming.Input) - The official/correct solution for xbox one controllers.
			//    * Has issuses with xbox 360 controllers
			//    * At the time of writing this has build issues with using experemental std libs and not supporting /std:c++latest
			//    * Requires a NuGet package
			//    * Doesnt allow background input
			//    * Only supported on some Windows versions (10/1703+?)
			//    * Does it support non-xbox controllers?
			// 4. Use RAWINPUT/hid directly. Read the hid reports using RAWINPUT.data.hid.bRawData or CreateFile/ReadFile/DeviceIoControl
			//    and manually parse the data. (Apparently bRawData is the raw hid report data)
			//    * Pain to implement
			//    * High maintenance
			//    * Will require lots of special cases to handle different controller features ("impulse triggers", PS4 rumble/led, etc.)
			//    * Only works for know devices since windows doesnt give us a way to query true hid report descriptors.
			//      We do have PHIDP_PREPARSED_DATA, but this is opaque and can only be read through HidP_*Caps
			//      functions which cant be trusted to report correct data (merged z axis for example).
			//      There are some libraries which claim to give report descriptors but from what i have seen
			//      they all actually use the presparsed data and caps info to build new report descriptors that
			//      do not truely match the originals given by the hid device and as such probably do not contain the correct
			//      z axis information.
			// 5. Use SDL
			//    * Probably the most robust and feature packed solution due to popularity/age of the project (apparently steam input uses this?)
			//    * Dont really want to pull in all of sdl just for controller input
			//
			// SDL has a number of input "drivers" you can use: xinput, winrt, rawinput, direct hid.
			// On its rawinput path it uses xinput and winrt to fill in the shortcomings of rawinput.
			//
			// It seems like the best solution atm might be to use XInput with RAWINPUT fallback and then
			// fill feature gaps (rumble, etc.) with WinRT? Is this possible? This only works if you can
			// match/pair the same device across all APIs.
			//
			// Links/Reading:
			// Windows Input Methods: https://github.com/MysteriousJ/Joystick-Input-Examples (https://web.archive.org/web/20220322234501/https://github.com/MysteriousJ/Joystick-Input-Examples)
			// RAWINPUT controller example: https://www.codeproject.com/Articles/185522/Using-the-Raw-Input-API-to-Process-Joystick-Input (https://web.archive.org/web/20210722131101/https://www.codeproject.com/Articles/185522/Using-the-Raw-Input-API-to-Process-Joystick-Input)
			// Chromium: https://chromium.googlesource.com/chromium/chromium/+/refs/heads/main/content/browser/gamepad/
			// Chromium: https://chromium.googlesource.com/chromium/src/+/refs/heads/main/device/gamepad/
			// SDL: https://github.com/libsdl-org/SDL/blob/main/src/hidapi/windows/hid.c
			// SDL Discussion on different APIs: https://discourse.libsdl.org/t/supporting-more-than-4-xinput-capable-devices-on-windows-rfc/25666 (https://web.archive.org/web/20220323003257/https://discourse.libsdl.org/t/supporting-more-than-4-xinput-capable-devices-on-windows-rfc/25666) (https://web.archive.org/web/20211116045707/https://discourse.libsdl.org/t/supporting-more-than-4-xinput-capable-devices-on-windows-rfc/25666?page=2)
			// SDL Controller Database: https://github.com/gabomdq/SDL_GameControllerDB
			// Understanding HID Report Descriptors: http://who-t.blogspot.com/2018/12/understanding-hid-report-descriptors.html (https://web.archive.org/web/20220323050817/http://who-t.blogspot.com/2018/12/understanding-hid-report-descriptors.html)
			// Windows Driver Kit samples "HClient" (search "WDK Samples"): https://github.com/microsoft/Windows-driver-samples/tree/master/hid/hclient
			//
			struct HIDState {
				std::vector<HIDP_BUTTON_CAPS> btnCaps;
				std::vector<HIDP_VALUE_CAPS> axisCaps;
			};

			static constexpr wchar_t className[] = L"Engine::Win32::OpenGLWindow";
			
			// TODO: make template after de-globalizing UI/ImGui
			WindowCallbackFunctions& callbacks;
			HWND windowHandle = nullptr;
			HDC deviceContext = nullptr;
			HGLRC renderContext = nullptr;
			bool close = false;
			bool mouseInWindow = false;

			// Must be 8 byte aligned because of RAWINPUT. See GetRawInputBuffer docs: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getrawinputbuffer
			// This size is more/less arbitrary. For both mouse/keyboard version of RAWINPUT we
			// can use a fixed sizeof(RAWINPUT), but for the hid version (controller/joystick/other) 
			// we still need potentially arbitrary size. I guess whenever we add hid version support
			// we should use a vector and resize when needed? Is there a better option? Maybe controllers have a maximal size?
			alignas(8) BYTE rawInputBuffer[128] = {};
			std::vector<byte> hidPreparsedData;

			// TODO: if we want to save key bindings to a config file we will need a way to back to HANDLE and use RIDI_DEVICENAME to save
			std::vector<HANDLE> deviceHandleToId;
			std::vector<std::unique_ptr<KeyboardState>> keyboardData;

		public:
			OpenGLWindow(const PixelFormat& pixelFormat, const ContextFormat& contextFormat, WindowCallbackFunctions& callbacks);
			OpenGLWindow(const OpenGLWindow&) = delete;

			~OpenGLWindow(); // TODO: cleanup Windows objects

			OpenGLWindow& operator=(const OpenGLWindow&) = delete;

			void show();

			void makeContextCurrent();

			void poll();

			void swapBuffers();

			void setSwapInterval(int interval);

			HWND getWin32WindowHandle() const;

			bool shouldClose() const;

			glm::ivec2 getFramebufferSize() const;

			// TODO: std::string getClipboardText() const;
			// TODO: void setClipboardText

			Input::DeviceId getDeviceId(HANDLE handle);

			void setPosition(int32 x, int32 y);
			void setSize(int32 w, int32 h);
			void setPosSize(int32 x, int32 y, int32 w, int32 h);
			void setClientArea(int32 w, int32 h);
			void center();

		private:
			void setPosSize(int32 x, int32 y, int32 w, int32 h, UINT flags);

			static WGLPointers init();

			static LRESULT windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

			template<UINT Msg>
			static LRESULT processMessage(OpenGLWindow& window, WPARAM wParam, LPARAM lParam) = delete;

			KeyboardState& getKeyboardState(Input::DeviceId id);
	};
}
