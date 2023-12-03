#pragma once


namespace Engine::Win32 {
	/**
	 * Gets the last Windows error message as a UTF-8 string.
	 */
	std::string getLastErrorMessage();

	class StartupInfo {
		public:
			std::wstring_view type = {};
			std::wstring_view debugging = {};
	};

	StartupInfo getStartupInfo();
}
