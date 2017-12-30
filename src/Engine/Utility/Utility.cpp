// STD
#include <fstream>

// Engine
#include <Engine/Utility/Utility.hpp>

namespace Engine::Utility {
	std::string readFile(const std::string& path) {
		std::ifstream file{path, std::ios::binary | std::ios::in};

		if (!file) {
			throw std::runtime_error{"Unable to read file \"" + path + "\""};
		}

		std::string content;
		file.seekg(0, std::ios::end);
		content.resize(file.tellg());
		file.seekg(0, std::ios::beg);
		file.read(content.data(), content.size());

		return content;
	}
}