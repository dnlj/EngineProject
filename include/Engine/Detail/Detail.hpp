#pragma once

// STD
#include <iostream>
#include <string>
#include <string_view>
#include <ostream>

namespace Engine::Detail {
	/**
	 * @brief Outputs the file, line, and prefix to an output stream.
	 * @param[in,out] out THe output stream.
	 * @param[in] prefix The prefix.
	 * @param[in] file The file.
	 * @param[in] line The line.
	 * @return @p out.
	 */
	std::ostream& log(std::ostream& out, std::string_view prefix, std::string_view file, int line);
}
