// STD
#include <fstream>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Utility/Utility.hpp>

namespace Engine::Utility {
	std::string readFile(const std::string& path) {
		std::ifstream file{path, std::ios::binary | std::ios::in};

		if (!file) {
			ENGINE_WARN("Unable to read file: ", path);
			return {};
		}

		std::string content;
		file.seekg(0, std::ios::end);
		content.resize(file.tellg());
		file.seekg(0, std::ios::beg);
		file.read(content.data(), content.size());

		return content;
	}
}
