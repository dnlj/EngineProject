#pragma once

// STD
#include <iostream>
#include <string>
#include <sstream>
#include <string_view>
#include <ostream>

#ifdef ENGINE_OS_WINDOWS
#include <io.h>
#endif

namespace Engine::Detail {
	// TODO: C++20: Use std <format> and fallback to operator<< if no format found.
	template<class... Args>
	void log(
		FILE* out,
		std::string_view prefix,
		std::string_view color,
		std::string_view file,
		int line,
		Args&&... args) {

		const auto date = getDateTimeString();
		const auto shortDate = std::string_view{date}.substr(11,8);
		const auto isTerminal = isatty(fileno(out));
		const auto useColor = isTerminal;
		const auto useShort = isTerminal;

		std::ostringstream stream;

		stream
			<< (useColor ? color : "")
			<< "[" << (useShort ? shortDate : date) << "]"
			<< "[" << file << ":" << line << "]"
			<< prefix
			<< (useColor ? Engine::ASCII_RESET : "")
			<< " ";
		(stream << ... << std::forward<Args>(args));
		stream << '\n';

		// TODO: C++20 ostringstream.view() to avoid copy
		fputs(stream.str().c_str(), out);

		// TODO: cmd line option for if should also log to file
		// TODO: when logging to file use full date and no color
	}

	std::string getDateTimeString();
}
