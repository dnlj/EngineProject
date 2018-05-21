#pragma once

// STD
#include <string>

namespace Engine::Utility {
	/**
	 * Gets the contents of the file located at @p path.
	 * @param[in] path The path to the file.
	 * @return The contents of the file as a string.
	 */
	std::string readFile(const std::string& path);
}
