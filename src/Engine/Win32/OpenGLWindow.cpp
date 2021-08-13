// Windows
#include <Windows.h>
#include <windowsx.h>

// STD
#include <iomanip>
#include <chrono>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Win32/Win32.hpp>
#include <Engine/Win32/OpenGLWindow.hpp>
#include <Engine/Clock.hpp>


namespace {
	using namespace Engine::Types;

	template<class T>
	T getFunctionPointerGL(const char* name) {
		auto addr = wglGetProcAddress(name);
		ENGINE_ASSERT(addr, "Unable to get WGL function pointer for ", name, " - ", Engine::Win32::getLastErrorMessage());
		return reinterpret_cast<T>(addr);

		// TODO: only needed for gl 1?
		// if (addr) { return addr; }
		//static auto module = [](){
		//	const auto handle = GetModuleHandleW(L"opengl32.dll");
		//	if (!handle) {
		//		abort(); // TODO: handle error. GetLastError
		//	}
		//	return  handle;
		//}();
		//
		//addr = GetProcAddress(module, name);
	}

	constexpr bool getKeyExtended(LPARAM lParam) {
		return lParam & (1 << 24);
	}
	
	constexpr int16 getKeyScancode(LPARAM lParam) {
		return ((lParam & 0xFF'00'00) >> 16);
	}

	constexpr bool getKeyRepeat(LPARAM lParam) {
		return lParam & (1 << 30);
	}

	uint16 getScancodeIndex(uint16 code) {
		const auto hi = code & 0xFF00;
		const auto lo = code & 0x00FF;
		switch (hi) {
			case 0x0000: return 0xFF * 0 + lo;
			case 0xE000: return 0xFF * 1 + lo;
			case 0xE100: return 0xFF * 2 + lo;
			case 0xAA00: return 0xFF * 3 + lo;
		}
		return 0;
	}

	uint16 getScancode(const RAWKEYBOARD& data) {
		const bool isE0 = data.Flags & RI_KEY_E0;
		const bool isE1 = data.Flags & RI_KEY_E1;
		ENGINE_DEBUG_ASSERT((isE0 ^ isE1) | !isE0, "Scancode extension E0 and E1 set");

		uint16 scancode = data.MakeCode | (isE0 ? 0xE000 : (isE1 ? 0xE100 : 0x0000));
		if (data.MakeCode == 0) {
			scancode = 0xAA00 | data.VKey;
		}

		return scancode;
	}

	void printRawMouse(const RAWINPUT& raw) {
		const auto data = raw.data.mouse;
		std::cout << "Raw Mouse Input: "
			<< "\n\tDevice: " << raw.header.hDevice
			<< "\n\tFlags: " << data.usFlags
			<< "\n\tButtons: " << data.ulButtons
			<< "\n\tButtonFlags: " << data.usButtonFlags
			<< "\n\tButtonData: " << data.usButtonData
			<< "\n\tRawButtons: " << data.ulRawButtons
			<< "\n\tLastX: " << data.lLastX
			<< "\n\tLastY: " << data.lLastY
			<< "\n\tExtraInformation: " << data.ulExtraInformation
			<< "\n";
	}

	void printRawKeyboard(const RAWINPUT& raw) {
		const auto data = raw.data.keyboard;
		const auto scancode = getScancode(data);
		const auto flags = std::cout.flags();
		std::cout
			<< "\nRaw: "
			<< "\n\tDevice: " << raw.header.hDevice
			<< "\n\tScancode: " << scancode
			<< " (0x" << std::setfill('0') << std::setw(4) << std::uppercase << std::hex << scancode << ")";
				
		std::cout.flags(flags);
		std::cout
			<< "\n\tMakeCode: " << data.MakeCode
			<< " (0x" << std::setfill('0') << std::setw(4) << std::uppercase << std::hex << data.MakeCode << ")";

		std::cout.flags(flags);
		std::cout
			<< "\n\tVKey: " << data.VKey
			<< " (0x" << std::setfill('0') << std::setw(4) << std::uppercase << std::hex << data.VKey << ")";

		std::cout.flags(flags);
		std::cout
			<< "\n\tVKey Map: " << MapVirtualKeyW(data.VKey, MAPVK_VK_TO_VSC)
			<< " (0x" << std::setfill('0') << std::setw(4) << std::uppercase << std::hex << MapVirtualKeyW(data.VKey, MAPVK_VK_TO_VSC) << ")";

		std::cout.flags(flags);
		std::cout
			<< "\n\tFlags: " << data.Flags
			<< "\n\tReserved: " << data.Reserved
			<< "\n\tMessage: " << data.Message
			<< "\n\tExtraInformation: " << data.ExtraInformation
			<< "\n";

		std::cout.flags(flags);
	}

	void printRawDevices() {
		UINT numDevices;
		ENGINE_ASSERT(GetRawInputDeviceList(nullptr, &numDevices, sizeof(RAWINPUTDEVICELIST)) != static_cast<UINT>(-1),
			"Unable to get number of input devices - ", Engine::Win32::getLastErrorMessage()
		);

		std::vector<RAWINPUTDEVICELIST> deviceList(numDevices);
		ENGINE_ASSERT(GetRawInputDeviceList(deviceList.data(), &numDevices, sizeof(RAWINPUTDEVICELIST)) != static_cast<UINT>(-1),
			"Unable to get input devices - ", Engine::Win32::getLastErrorMessage()
		);

		std::cout << "Device List - numDevices: " << numDevices;
		RID_DEVICE_INFO info = {.cbSize = sizeof(RID_DEVICE_INFO)};
		wchar_t name[256] = {};
		for (uint32 i = 0; i < numDevices; ++i) {
			const auto& dev = deviceList[i];

			UINT size = std::extent_v<decltype(name)> - 1;
			size = GetRawInputDeviceInfoW(dev.hDevice, RIDI_DEVICENAME, &name, &size);

			std::cout << "\n  " << i << ": "
				<< "\n    Type: " << dev.dwType
				<< " (" << ((dev.dwType == RIM_TYPEMOUSE) ? "RIM_TYPEMOUSE" : ((dev.dwType == RIM_TYPEKEYBOARD) ? "RIM_TYPEKEYBOARD" : ((dev.dwType == RIM_TYPEHID) ? "RIM_TYPEHID" : ("Unknown")))) << ")"
				<< "\n    Handle: " << dev.hDevice
				<< "\n    Name Size: " << size;
			std::wcout
				<< "\n    Name: " << name;

			size = sizeof(info);
			size = GetRawInputDeviceInfoW(dev.hDevice, RIDI_DEVICEINFO, &info, &size);
			ENGINE_DEBUG_ASSERT(dev.dwType == info.dwType);

			if (dev.dwType == RIM_TYPEMOUSE) {
				std::cout
					<< "\n    Id: " << info.mouse.dwId
					<< "\n    NumberOfButtons: " << info.mouse.dwNumberOfButtons
					<< "\n    SampleRate: " << info.mouse.dwSampleRate
					<< "\n    HasHorizontalWheel: " << info.mouse.fHasHorizontalWheel;
			} else if (dev.dwType == RIM_TYPEKEYBOARD) {
				std::cout
					<< "\n    Type: " << info.keyboard.dwType
					<< "\n    SubType: " << info.keyboard.dwSubType
					<< "\n    KeyboardMode: " << info.keyboard.dwKeyboardMode
					<< "\n    NumberOfFunctionKeys: " << info.keyboard.dwNumberOfFunctionKeys
					<< "\n    NumberOfIndicators: " << info.keyboard.dwNumberOfIndicators
					<< "\n    NumberOfKeysTotal: " << info.keyboard.dwNumberOfKeysTotal;
			} else if (dev.dwType == RIM_TYPEHID) {
				auto flags = std::cout.flags();
				std::cout
					<< "\n    VendorId: " << info.hid.dwVendorId
					<< "\n    ProductId: " << info.hid.dwProductId
					<< "\n    VersionNumber: " << info.hid.dwVersionNumber
					<< "\n    UsagePage: " << info.hid.usUsagePage
					<< " (0x" << std::setfill('0') << std::setw(4) << std::uppercase << std::hex << info.hid.usUsagePage << ")"
					<< "\n    Usage: " << info.hid.usUsage
					<< " (0x" << std::setfill('0') << std::setw(4) << std::uppercase << std::hex << info.hid.usUsage << ")";
				std::cout.flags(flags);
			}
		}
		std::cout << "\n";
	}
}

namespace Engine::Win32 {
	template<>
	LRESULT OpenGLWindow::processMessage<WM_DESTROY>(OpenGLWindow& window, WPARAM wParam, LPARAM lParam) {
		PostQuitMessage(0);
		return 0;
	}

	template<>
	LRESULT OpenGLWindow::processMessage<WM_SIZE>(OpenGLWindow& window, WPARAM wParam, LPARAM lParam) {
		const int32 w = LOWORD(lParam);
		const int32 h = HIWORD(lParam);
		window.callbacks.resizeCallback(w, h);
		return 0;
	}

	template<>
	LRESULT OpenGLWindow::processMessage<WM_CLOSE>(OpenGLWindow& window, WPARAM wParam, LPARAM lParam) {
		window.close = true;
		return 0;
	}

	template<>
	LRESULT OpenGLWindow::processMessage<WM_INPUT>(OpenGLWindow& window, WPARAM wParam, LPARAM lParam) {
		// Useful links
		// MS docs: https://docs.microsoft.com/en-us/windows/win32/inputdev/using-raw-input
		// USB HID usage pages and ids: https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf
		// USB HID: https://www.usb.org/hid
		// MS usage pages: https://docs.microsoft.com/en-us/windows-hardware/drivers/hid/top-level-collections-opened-by-windows-for-system-use
		// Scancode info: https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html and https://www.win.tue.nl/~aeb/linux/kbd/scancodes.html
		// USB Vendor Ids: http://www.linux-usb.org/usb.ids and http://www.linux-usb.org/usb-ids.html
		// If raw.hDevice == 0 then it is a virtual device such as On-Screen Keyboard or other software.
		UINT size = std::extent_v<decltype(rawInputBuffer)>;

		if constexpr (ENGINE_DEBUG) {
			GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));
			ENGINE_ASSERT(size <= std::extent_v<decltype(rawInputBuffer)>, "Raw input buffer to small. Needs at least ", size);
		}

		// TODO: dont we need to check for buffer size? See ms docs for GetRawInputData
		GetRawInputData(
			reinterpret_cast<HRAWINPUT>(lParam),
			RID_INPUT,
			window.rawInputBuffer,
			&size,
			sizeof(RAWINPUTHEADER)
		);

		const auto& raw = *reinterpret_cast<RAWINPUT*>(window.rawInputBuffer);

		// TODO: split into functions like we did for WM_* messages.
		if (raw.header.dwType == RIM_TYPEMOUSE) {
			// If the cursor is visible we should use WM_MOUSEMOVE so we maintain cursor ballistics.
			// Although if you use WM_MOUSEMOVE you cannot distinguish between multiple mice.
			// printRawMouse(raw);
			//ENGINE_LOG("RIM_TYPEMOUSE");

			// TODO: switch to this atleast for mouse button presses. Not sure why we dont already.
			const auto& data = raw.data.mouse;
			static_assert(sizeof(data.usButtonFlags) <= sizeof(int));
			int buttons = data.usButtonFlags & ~(RI_MOUSE_WHEEL | RI_MOUSE_HWHEEL); // Don't process mouse wheel events

			// Background event
			if (wParam == RIM_INPUTSINK || !window.mouseInWindow) {
				// Only process release events when in the background
				buttons &= RI_MOUSE_BUTTON_1_UP | RI_MOUSE_BUTTON_2_UP | RI_MOUSE_BUTTON_3_UP | RI_MOUSE_BUTTON_4_UP | RI_MOUSE_BUTTON_5_UP;
			} else {
				// Only process cursor events in foreground
				// TODO: impl mouse delta when cursor hidden. FPS games for example.
				//ENGINE_LOG("Move: ", data.usFlags, " ", data.lLastX, " ", data.lLastY);
			}

			const auto device = window.getDeviceId(raw.header.hDevice);
			const auto time = Clock::TimePoint{std::chrono::milliseconds{GetMessageTime()}};

			if (buttons) {
				// Windows only recognizes up to five mouse buttons. See RAWMOUSE struct and RI_MOUSE_BUTTON_* constants.
				// https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-rawmouse
				for (int i = 0; i < 5*2; ++i) { // Five buttons * press/release
					if (buttons & (1 << i)) {
						const Input::InputEvent event = {
							.state = {
								.id = { .type = Input::InputType::MOUSE, .device = device, .code = static_cast<uint16>(i >> 1) },
								.value = { .i32 = !(i & 1) },
							},
							.time = time,
						};
						window.callbacks.mouseButtonCallback(event);
					}
				}
			}

			if (data.usButtonData) {
				float32 scroll = static_cast<float32>(static_cast<SHORT>(data.usButtonData));
				scroll /= WHEEL_DELTA;
				
				Input::InputEvent event = {
					.state = {
						.id = { .type = Input::InputType::MOUSE_WHEEL, .device = device, .code = 0 },
						.value = { .f32 = scroll },
					},
					.time = time,
				};

				if (data.usButtonFlags & RI_MOUSE_WHEEL) {
					window.callbacks.mouseWheelCallback(event);
				} else if (data.usButtonFlags & RI_MOUSE_HWHEEL) {
					event.state.id.code = 1;
					window.callbacks.mouseWheelCallback(event);
				}
			}
		} else if (raw.header.dwType == RIM_TYPEKEYBOARD) {
			const auto device = window.getDeviceId(raw.header.hDevice);
			const auto& data = raw.data.keyboard;
			const auto scancode = getScancode(data);
			auto& wasPressed = window.getKeyboardState(device).state[getScancodeIndex(scancode)];
			const bool pressed = !(data.Flags & RI_KEY_BREAK);
			
			// Don't record background presses
			if (wParam == RIM_INPUTSINK && (pressed || !wasPressed)) { return 0; }

			if (!pressed || !wasPressed) {
				wasPressed = pressed;

				const Input::InputEvent event = {
					.state = {
						.id = {Input::InputType::KEYBOARD, device, scancode},
						.value = { .i32 = pressed },
					},
					.time = Clock::TimePoint{std::chrono::milliseconds{GetMessageTime()}}
				};

				window.callbacks.keyCallback(event);
			}
		} else if (raw.header.dwType == RIM_TYPEHID) {
			// TODO: Gamepad input
		}

		return 0;
	}

	template<>
	LRESULT OpenGLWindow::processMessage<WM_CHAR>(OpenGLWindow& window, WPARAM wParam, LPARAM lParam) {
		window.callbacks.charCallback(static_cast<wchar_t>(wParam));
		return 0;
	}

	template<>
	LRESULT OpenGLWindow::processMessage<WM_MOUSEMOVE>(OpenGLWindow& window, WPARAM wParam, LPARAM lParam) {
		if (!window.mouseInWindow) {
			TRACKMOUSEEVENT event = {
				.cbSize = sizeof(event),
				.dwFlags = TME_LEAVE,
				.hwndTrack = window.getWin32WindowHandle(),
				.dwHoverTime = 0,
			};
			TrackMouseEvent(&event);
			window.callbacks.mouseEnterCallback();
			window.mouseInWindow = true;
		}

		const int32 x = GET_X_LPARAM(lParam);
		const int32 y = GET_Y_LPARAM(lParam);

		//ENGINE_LOG("Move: ", x, " ", y);

		// TODO: we currently don't distinguish between mouse devices
		const Input::InputEvent event = {
			.state = {
				.id = {Input::InputType::MOUSE_AXIS, 0, 0},
				.value = { .f32v2 = {x , y} },
			},
			.time = Clock::TimePoint{std::chrono::milliseconds{GetMessageTime()}}
		};
		window.callbacks.mouseMoveCallback(event);

		return 0;
	}

	template<>
	LRESULT OpenGLWindow::processMessage<WM_MOUSELEAVE>(OpenGLWindow& window, WPARAM wParam, LPARAM lParam) {
		//
		// BUG: If you press a mouse button and then move the mouse on to
		// an overlapping window without leaving the current window (alt tabbing for example)
		// we will not receive a WM_MOUSELEAVE until the button has been
		// released AND we move the mouse.
		//
		// Multiple people have claimed that this is because the
		// default handlers for WM_*MOUSE* press events call SetCapture. However,
		// even if we handle all WM_*MOUSE* events and do not call SetCapture
		// this behaviour still occurs.
		//
		// The solution for this would be to handle all WM_*MOUSE* press/release events (or handle them in WM_INPUT)
		// and manually call SetCapture/ReleaseCapture so that we get WM_MOUSEMOVE
		// events even when the mouse is not in our window. After that we would then
		// get rid of WM_MOUSELEAVE/TrackMouseEvent/TME_LEAVE all together and
		// manually check the cursor position againt our client area.
		//

		window.mouseInWindow = false;
		window.callbacks.mouseLeaveCallback();
		return 0;
	}
}

////////////////////////////////////////////////////////////////////////////////
// OpenGLWindow
////////////////////////////////////////////////////////////////////////////////
namespace Engine::Win32 {
	OpenGLWindow::OpenGLWindow(const PixelFormat& pixelFormat, const ContextFormat& contextFormat, WindowCallbackFunctions& callbacks)
		: callbacks{callbacks} {
		wglPtrs = OpenGLWindow::init();

		windowHandle = CreateWindowExW(
			0, // TODO:
			className,
			(std::wstring{L"My Window Title - "} + (ENGINE_SERVER?L"Server":L"Client")).c_str(),
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
			//WS_POPUP,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			1280,
			720,
			0,
			0,
			GetModuleHandleW(nullptr),
			nullptr
		);
		ENGINE_ASSERT(windowHandle, "Unable to create window. - ", getLastErrorMessage());
		SetWindowLongPtrW(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

		deviceContext = GetDC(windowHandle);
		ENGINE_ASSERT(deviceContext, "Unable to get Windows device context - ", getLastErrorMessage());

		// TODO: srgb?
		// TODO: figure out what we actually want
		const int pixelAttributes[] = {
			WGL_DRAW_TO_WINDOW_ARB, true,
			WGL_SUPPORT_OPENGL_ARB, true,
			WGL_DOUBLE_BUFFER_ARB, true,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB, pixelFormat.colorBits,
			WGL_ALPHA_BITS_ARB, pixelFormat.alphaBits,
			WGL_DEPTH_BITS_ARB, pixelFormat.depthBits,
			WGL_STENCIL_BITS_ARB, pixelFormat.stencilBits,
			//WGL_SAMPLE_BUFFERS_ARB, true,
			//WGL_SAMPLES_ARB, 4,
			0
		};

		int winPixelFormat;
		UINT numFormats;
	
		// TODO: glfw manually chooses pixel format - wglGetPixelFormatAttribivARB, WGL_NUMBER_PIXEL_FORMATS_ARB
		// TODO: do we want multiple results?
		// TODO: verify chosen pixel format
		ENGINE_ASSERT(wglPtrs.wglChoosePixelFormatARB(deviceContext, pixelAttributes, nullptr, 1, &winPixelFormat, &numFormats),
			"Unable to find suitable pixel format - ", getLastErrorMessage()
		);

		PIXELFORMATDESCRIPTOR pixelFormatDesc;
		ENGINE_ASSERT(DescribePixelFormat(deviceContext, winPixelFormat, sizeof(pixelFormatDesc), &pixelFormatDesc),
			"Unable to find describe pixel format - ", getLastErrorMessage()
		);

		ENGINE_ASSERT(SetPixelFormat(deviceContext, winPixelFormat, &pixelFormatDesc),
			"Unable to set pixel format - ", getLastErrorMessage()
		);

		// TODO: figure out what we actually want
		const int contextAttributes[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, contextFormat.majorVersion,
			WGL_CONTEXT_MINOR_VERSION_ARB, contextFormat.minorVersion,
			WGL_CONTEXT_FLAGS_ARB, contextFormat.debug ? WGL_CONTEXT_DEBUG_BIT_ARB : 0,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0
		};
	
		renderContext = wglPtrs.wglCreateContextAttribsARB(deviceContext, nullptr, contextAttributes);
		ENGINE_ASSERT(renderContext, "Unable to create WGL render context - ", getLastErrorMessage());

		{ // Setup device ids
			UINT numDevices;
			ENGINE_ASSERT(GetRawInputDeviceList(nullptr, &numDevices, sizeof(RAWINPUTDEVICELIST)) != static_cast<UINT>(-1),
				"Unable to get number of input devices - ", getLastErrorMessage()
			);

			std::vector<RAWINPUTDEVICELIST> deviceList(numDevices);
			ENGINE_ASSERT(GetRawInputDeviceList(deviceList.data(), &numDevices, sizeof(RAWINPUTDEVICELIST)) != static_cast<UINT>(-1),
				"Unable to get input devices - ", getLastErrorMessage()
			);

			// On-Screen Keyboard and other software inputs use device 0
			deviceHandleToId.reserve(numDevices + 1);
			keyboardData.resize(numDevices + 1);
			deviceHandleToId.push_back(0);
			keyboardData[0].reset(new KeyboardState{});

			size_t lastKeyboard = 0;
			for (const auto& dev : deviceList) {
				const auto id = deviceHandleToId.size();
				deviceHandleToId.push_back(dev.hDevice);

				if (dev.dwType == RIM_TYPEKEYBOARD) {
					keyboardData[id].reset(new KeyboardState{});
					lastKeyboard = id;
				}
			}

			keyboardData.resize(lastKeyboard + 1);
			deviceHandleToId.shrink_to_fit();
			keyboardData.shrink_to_fit();
		}

		{ // Register raw input
			RAWINPUTDEVICE devices[] = {
				{ // Mouse
					.usUsagePage = 0x01,
					.usUsage = 0x02,
					.dwFlags = RIDEV_INPUTSINK,  // We still need legacy messages for correct cursor w/ ballistics
					.hwndTarget = windowHandle,
				},
				// TODO: Gamepad
				//{ // Gamepad
				//	.usUsagePage = 0x01,
				//	.usUsage = 0x05,
				//	.dwFlags = 0,
				//	.hwndTarget = nullptr,
				//},
				{ // Keyboard
					.usUsagePage = 0x01,
					.usUsage = 0x06,
					.dwFlags = RIDEV_INPUTSINK, // TODO: RIDEV_NOLEGACY once we have scancode -> unicode to replace WM_CHAR
					.hwndTarget = windowHandle,
				},
			};

			if (!RegisterRawInputDevices(devices, std::extent_v<decltype(devices)>, sizeof(devices[0]))) {
				ENGINE_ERROR("Unable to register input devices - ", getLastErrorMessage());
			}
		}
	}

	OpenGLWindow::~OpenGLWindow() {
		ENGINE_ASSERT(wglMakeCurrent(nullptr, nullptr), "Unable to make WGL render context non-current - ", getLastErrorMessage());
		ENGINE_ASSERT(wglDeleteContext(renderContext), "Unable to delete WGL render context - ", getLastErrorMessage());
		ReleaseDC(windowHandle, deviceContext);
		ENGINE_ASSERT(DestroyWindow(windowHandle), "Unable to destroy window for - ", getLastErrorMessage());
	}

	void OpenGLWindow::show() {
		ShowWindow(windowHandle, SW_SHOW);
	}

	void OpenGLWindow::makeContextCurrent() {
		wglMakeCurrent(deviceContext, renderContext);
	}

	void OpenGLWindow::poll() {
		for (MSG msg; PeekMessageW(&msg, windowHandle, 0, 0, PM_REMOVE);) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	void OpenGLWindow::swapBuffers() {
		SwapBuffers(deviceContext);
	}

	void OpenGLWindow::setSwapInterval(int interval) {
		if (!wglPtrs.wglSwapIntervalEXT(interval)) {
			ENGINE_WARN("Unable to set swap interval to ", interval, ". Current swap interval is ", wglPtrs.wglGetSwapIntervalEXT(), ".");
		}
	}

	HWND OpenGLWindow::getWin32WindowHandle() const {
		return windowHandle;
	}

	bool OpenGLWindow::shouldClose() const {
		return close;
	}

	glm::ivec2 OpenGLWindow::getFramebufferSize() const {
		RECT rect;
		GetClientRect(windowHandle, &rect);
		return glm::ivec2{rect.right, rect.bottom};
	}

	Input::DeviceId OpenGLWindow::getDeviceId(HANDLE handle) {
		const auto max = static_cast<Input::DeviceId>(deviceHandleToId.size());
		for (Input::DeviceId i = 0; i < max; ++i) {
			if (handle == deviceHandleToId[i]) {
				return i;
			}
		}

		[[unlikely]]
		ENGINE_WARN("Unknown hardware device <-> id mapping (", handle, ")");
		return 0;
	}

	
	auto OpenGLWindow::getKeyboardState(Input::DeviceId id) -> KeyboardState& {
		return *keyboardData[id];
	}

	void OpenGLWindow::setPosition(int32 x, int32 y) {
		setPosSize(x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}

	void OpenGLWindow::setSize(int32 w, int32 h) {
		setPosSize(0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER);
	}

	void OpenGLWindow::setPosSize(int32 x, int32 y, int32 w, int32 h) {
		setPosSize(x, y, w, h, SWP_NOZORDER);
	}

	void OpenGLWindow::setPosSize(int32 x, int32 y, int32 w, int32 h, UINT flags) {
		RECT off = {};
		AdjustWindowRectEx(&off, GetWindowStyle(windowHandle), false,  GetWindowExStyle(windowHandle));
		SetWindowPos(windowHandle, HWND_TOP,
			x + off.left,
			y - 1,
			w - off.left + off.right,
			h + off.bottom + 1,
			flags
		);
	}

	void OpenGLWindow::setClientArea(int32 w, int32 h) {
		RECT off = {};
		AdjustWindowRectEx(&off, GetWindowStyle(windowHandle), false,  GetWindowExStyle(windowHandle));
		SetWindowPos(windowHandle, HWND_TOP, 0,0,
			w - off.left + off.right,
			h - off.top + off.bottom,
			SWP_NOMOVE | SWP_NOZORDER
		);
	}

	void OpenGLWindow::center() {
		RECT selfRect;
		GetWindowRect(windowHandle, &selfRect);

		const auto monitor = MonitorFromWindow(windowHandle, MONITOR_DEFAULTTONEAREST);
		MONITORINFO minfo {
			.cbSize = sizeof(minfo),
			.rcMonitor = {.right = 640, .bottom = 480},
		};
		ENGINE_ASSERT_WARN(GetMonitorInfoW(monitor, &minfo), "Unable to determine monitor for window - ", getLastErrorMessage());

		setPosition(
			(minfo.rcMonitor.right - minfo.rcMonitor.left - selfRect.right + selfRect.left) / 2,
			(minfo.rcMonitor.bottom - minfo.rcMonitor.top - selfRect.bottom + selfRect.top) / 2
		);
	}

	auto OpenGLWindow::init() -> WGLPointers {
		const auto hInstance = GetModuleHandleW(nullptr);

		const WNDCLASSEXW windowClass {
			.cbSize = sizeof(windowClass),
			.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
			.lpfnWndProc = &windowProc,
			.cbClsExtra = 0,
			.cbWndExtra = 0,
			.hInstance = hInstance,
			.hIcon = 0,
			.hCursor = 0,
			.hbrBackground = 0,
			.lpszMenuName = nullptr,
			.lpszClassName = className,
			.hIconSm = 0,
		};
		ENGINE_ASSERT(RegisterClassExW(&windowClass), "Unable to register OpenGLWindow's WNDCLASSEX - ", getLastErrorMessage());

		const auto tempWindow = CreateWindowExW(
			0, // TODO:
			className,
			L"Temp window for gl loading",
			WS_OVERLAPPEDWINDOW, // TODO:
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			hInstance,
			nullptr
		);
		ENGINE_ASSERT(tempWindow, "Unable to create temporary window for wgl function loading - ", getLastErrorMessage());

		const PIXELFORMATDESCRIPTOR tempPixelFormatDesc{
			.nSize = sizeof(tempPixelFormatDesc),
			.nVersion = 1,
			.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
			.iPixelType = PFD_TYPE_RGBA,
			.cColorBits = 24,
			.cRedBits = 0,
			.cRedShift = 0,
			.cGreenBits = 0,
			.cGreenShift = 0,
			.cBlueBits = 0,
			.cBlueShift = 0,
			.cAlphaBits = 0,
			.cAlphaShift = 0,
			.cAccumBits = 0,
			.cAccumRedBits = 0,
			.cAccumGreenBits = 0,
			.cAccumBlueBits = 0,
			.cAccumAlphaBits = 0,
			.cDepthBits = 24,
			.cStencilBits = 8,
			.cAuxBuffers = 0,
			.iLayerType = 0,
			.bReserved = 0,
			.dwLayerMask = 0,
			.dwVisibleMask = 0,
			.dwDamageMask = 0,
		};

		const auto tempDeviceContext = GetDC(tempWindow);
		ENGINE_ASSERT(tempDeviceContext, "Unable to get Windows device context - ", getLastErrorMessage());

		const auto tempPixelFormat = ChoosePixelFormat(tempDeviceContext, &tempPixelFormatDesc);
		ENGINE_ASSERT(tempPixelFormat, "Unable to get Windows pixel format - ", getLastErrorMessage());

		ENGINE_ASSERT(SetPixelFormat(tempDeviceContext, tempPixelFormat, &tempPixelFormatDesc),
			"Unable to set Windows pixel format - ", getLastErrorMessage()
		);

		const auto tempRenderContext = wglCreateContext(tempDeviceContext);
		ENGINE_ASSERT(tempRenderContext, "Unable to create WGL render context - ", getLastErrorMessage());

		ENGINE_ASSERT(wglMakeCurrent(tempDeviceContext, tempRenderContext),
			"Unable to make WGL render context current - ", getLastErrorMessage()
		);

		WGLPointers pointers = {
			// If you ever have issues getting a function pointer make sure to check caps. EXT not Ext etc.
			.wglChoosePixelFormatARB = getFunctionPointerGL<PFNWGLCHOOSEPIXELFORMATARBPROC>("wglChoosePixelFormatARB"),
			.wglCreateContextAttribsARB = getFunctionPointerGL<PFNWGLCREATECONTEXTATTRIBSARBPROC>("wglCreateContextAttribsARB"),
			.wglSwapIntervalEXT = getFunctionPointerGL<PFNWGLSWAPINTERVALEXTPROC>("wglSwapIntervalEXT"),
			.wglGetSwapIntervalEXT = getFunctionPointerGL<PFNWGLGETSWAPINTERVALEXTPROC>("wglGetSwapIntervalEXT"),
		};
		
		ENGINE_ASSERT(wglMakeCurrent(nullptr, nullptr), "Unable to make WGL render context non-current - ", getLastErrorMessage());
		ENGINE_ASSERT(wglDeleteContext(tempRenderContext), "Unable to delete temporary WGL render context - ", getLastErrorMessage());
		ReleaseDC(tempWindow, tempDeviceContext);
		ENGINE_ASSERT(DestroyWindow(tempWindow), "Unable to destroy temporary window for WGL function loading - ", getLastErrorMessage());

		return pointers;
	}
	
	LRESULT OpenGLWindow::windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		#define HANDLE_MESSAGE(Msg) case Msg: {\
			return processMessage<Msg>(\
				*reinterpret_cast<OpenGLWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA)),\
				wParam, lParam\
			);\
		}

		switch (uMsg) {
			HANDLE_MESSAGE(WM_DESTROY);
			HANDLE_MESSAGE(WM_SIZE);
			HANDLE_MESSAGE(WM_CLOSE);
			HANDLE_MESSAGE(WM_INPUT);
			HANDLE_MESSAGE(WM_CHAR);
			HANDLE_MESSAGE(WM_MOUSEMOVE);
			HANDLE_MESSAGE(WM_MOUSELEAVE);
		}
		#undef HANDLE_MESSAGE

		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}
}
