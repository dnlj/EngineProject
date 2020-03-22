// STD
#include <string>
#include <ctime>
#include <iomanip>

// Engine
#include <Engine/Detail/Detail.hpp>

namespace {
	// TODO: look into C++20 chrono and std::formatter/<format>
	std::string getDateTimeString() {
		// Example output: 2017-12-24 18:29:35 -0600
		const auto time = std::time(nullptr);
		std::string date(26, '.');
		
		std::strftime(date.data(), date.size(), "%Y-%m-%d %H:%M:%S %z", localtime(&time));
		date.pop_back();
		return date;
	}
}

namespace Engine::Detail {
	std::ostream& log(std::ostream& out, std::string_view prefix, std::string_view file, int line) {
		const auto date = getDateTimeString();

		out
			<< "[" << date << "]"
			<< "[" << file << ":" << line << "]"
			<< prefix << " ";

		return out;
	}
}
