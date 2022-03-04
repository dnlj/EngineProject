// STD
#include <string>
#include <ctime>
#include <iomanip>

// Engine
#include <Engine/Detail/Detail.hpp>
#include <Engine/Input/InputId.hpp>


namespace Engine {
	void LogFormatter<Input::InputId>::format(std::ostream& stream, const Input::InputId& val) {
		stream << "InputId(" << (int)val.device << ", " << (int)val.type << ", " << (int)val.code << ")";
	}
}

namespace Engine::Detail {
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
