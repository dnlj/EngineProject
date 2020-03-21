#pragma once

namespace Engine::Windows {
	/**
	 * Gets the last Windows error message as a UTF-8 string.
	 */
	std::string getLastErrorMessage();
}
