// STD
#include <iostream>
#include <fstream>

// Engine
#include <Engine/ConfigParser.hpp>


namespace Engine {
	void ConfigParser::print() {
		std::cout << "=================================================\n";
		std::cout << toString();
		std::cout << "=================================================\n";
	}

	void ConfigParser::save(const std::string& path) const {
		std::fstream file{path, std::ios::out | std::ios::binary};

		if (!file) {
			ENGINE_WARN("Unable to write config to \"", path, "\"");
			return;
		}

		const auto& str = toString();
		file.write(str.data(), str.size());
	}
}
