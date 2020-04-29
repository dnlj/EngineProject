#pragma once

// STD
#include <iostream>
#include <string>
#include <string_view>
#include <ostream>

#ifdef ENGINE_OS_WINDOWS
#include <io.h>
#endif

namespace Engine::Detail {
	class LogTarget {
		public:
			std::ostream& stream;
			FILE* file;
	};

	inline LogTarget StandardOut = {std::cout, stdout};
	inline LogTarget StandardErr = {std::cerr, stderr};

	// TODO: C++20: get rid of streams&LogTarget and just use file handles and std <format>
	template<class... Args>
	std::ostream& log(
		LogTarget& out,
		std::string_view prefix,
		std::string_view color,
		std::string_view file,
		int line,
		Args&&... args) {

		const auto date = getDateTimeString();
		const auto useColor = color.size() > 0 && isatty(fileno(out.file));

		// TODO: shorten date-time output if isatty
		out.stream
			<< (useColor ? color : "")
			<< "[" << date << "]"
			<< "[" << file << ":" << line << "]"
			<< prefix
			<< (useColor ? Engine::ASCII_RESET : "")
			<< " ";

		(out.stream << ... << std::forward<Args>(args));

		out.stream << '\n';

		// TODO: cmd line option for if should also log to file

		return out.stream;
	}

	std::string getDateTimeString();
}
