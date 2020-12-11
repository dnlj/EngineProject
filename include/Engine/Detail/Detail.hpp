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

// Engine
#include <Engine/ASCIIColorString.hpp>
#include <Engine/GlobalConfig.hpp>
#include <Engine/Constants.hpp>

namespace Engine::Detail {
	std::string getDateTimeString();

	// TODO: C++20: Use std <format> and fallback to operator<< if no format found.
	template<class... Args>
	void log(
		std::string_view prefix,
		ASCIIColorString color,
		std::string_view file,
		int line,
		Args&&... args) {

		const auto& gc = Engine::getGlobalConfig();
		const auto date = getDateTimeString();

		std::ostringstream stream;
		const auto&& filter = [&](auto&& value){
			using V = decltype(value);
			if constexpr (std::is_same_v<std::decay_t<V>, ASCIIColorString>) {
				if (gc.logColor) {
					return value.str;
				} else {
					return "";
				}
			} else {
				return std::forward<V>(value);
			}
		};

		stream
			<< filter(color)
			<< "[" << (gc.logTimeOnly ? std::string_view{date}.substr(11,8) : date) << "]"
			<< "[" << file << ":" << line << "]"
			<< prefix
			<< filter(Engine::ASCII_RESET)
			<< " ";
		(stream << ... << filter(std::forward<Args>(args)));
		stream << '\n';

		// TODO: C++20 ostringstream.view() to avoid copy
		fputs(stream.str().c_str(), gc.log.get());

		// TODO: cmd line option for if should also log to file
		// TODO: when logging to file use full date and no color
	}

}
