// Windows
#include <Windows.h>

// STD
#include <string>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Win32/Win32.hpp>
#include <Engine/Clock.hpp>


namespace {
	const int _ENGINE_CLOCK_CHECK = [](){
		const auto tc = GetTickCount();
		const auto ec = Engine::Clock::now();
		const auto wc = Engine::Clock::TimePoint{std::chrono::milliseconds{tc}};

		// Verify that GetTickCount and Engine::Clock share an epoch.
		// If they are within a second they probably do.
		ENGINE_DEBUG_ASSERT((ec - wc) < std::chrono::seconds{1},
			"Engine::Clock is incompatible with Win32 GetTickCount."
		);

		return 0;
	}();
}


namespace Engine::Win32 {
	std::string getLastErrorMessage() {
		WCHAR* wmsg = nullptr;
		if (!FormatMessageW(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr,
				GetLastError(),
				0,
				reinterpret_cast<LPWSTR>(&wmsg),
				0,
				nullptr
			)) {
			ENGINE_ERROR("Unable to format Windows error message");
		};

		const auto size = WideCharToMultiByte(CP_UTF8, 0, wmsg, -1, nullptr, 0, nullptr, nullptr);
		std::string msg(size, '?');
		if (!WideCharToMultiByte(CP_UTF8, 0, wmsg, -1, msg.data(), size, nullptr, nullptr)) {
			ENGINE_ERROR("Unable to convert Windows error message to UTF-8");
		};

		if (LocalFree(wmsg) == wmsg) {
			ENGINE_ERROR("Unable to free Windows memory");
		}

		return msg;
	}
}
