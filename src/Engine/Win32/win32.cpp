// Windows
#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>

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

	StartupInfo getStartupInfo() {
		StartupInfo results = {
			.type = ENGINE_SERVER ? L"Server" : L"Client",
			.debugging = IsDebuggerPresent() ? L"Debugging" : L"Standalone",
		};

		// TODO: some reason this doesn't work, can't be bothered to do this correctly right now.
		//const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		//if (snapshot == INVALID_HANDLE_VALUE) {
		//	ENGINE_WARN2("Unable to check processes: {}", getLastErrorMessage());
		//} else {
		//	PROCESSENTRY32W entry{};
		//	entry.dwSize = sizeof(entry);
		//
		//	if (!Process32FirstW(snapshot, &entry)) {
		//		ENGINE_WARN2("Unable to check first process: {}", getLastErrorMessage());
		//	} else {
		//		do {
		//			//std::cout << "Process:\n";
		//			//std::cout << "\tdwSize: " << entry.dwSize << "\n";
		//			//std::cout << "\tcntUsage: " << entry.cntUsage << "\n";
		//			//std::cout << "\tth32ProcessID: " << entry.th32ProcessID << "\n";
		//			//std::cout << "\tth32DefaultHeapID: " << entry.th32DefaultHeapID << "\n";
		//			//std::cout << "\tth32ModuleID: " << entry.th32ModuleID << "\n";
		//			//std::cout << "\tcntThreads: " << entry.cntThreads << "\n";
		//			//std::cout << "\tth32ParentProcessID: " << entry.th32ParentProcessID << "\n";
		//			//std::cout << "\tpcPriClassBase: " << entry.pcPriClassBase << "\n";
		//			//std::cout << "\tdwFlags: " << entry.dwFlags << "\n";
		//			//std::wcout << "\tszExeFile: " << std::wstring_view{entry.szExeFile} << "\n";
		//
		//			// TODO: This isn't 100% correct because we only check against the
		//			// exe name which could easily collide with other programs. But I
		//			// can't be bothered to do this correctly atm.
		//			results.instance += std::wstring_view{entry.szExeFile} == results.type;
		//		} while (Process32NextW(snapshot, &entry));
		//	}
		//
		//	CloseHandle(snapshot);
		//}

		return results;
	}
}
