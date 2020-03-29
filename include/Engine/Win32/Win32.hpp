#pragma once


namespace Engine::Win32 {
	/**
	 * Gets the last Windows error message as a UTF-8 string.
	 */
	std::string getLastErrorMessage();
}
