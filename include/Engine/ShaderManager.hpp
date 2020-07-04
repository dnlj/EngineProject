#pragma once

// STD
#include <unordered_map>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// Engine
#include <Engine/ResourceManager.hpp>

// TODO: Document
namespace Engine {
	class ShaderManager : public Engine::ResourceManager<ShaderManager, GLuint> {
		friend class Engine::ResourceManager<ShaderManager, GLuint>;
		private:
			GLuint load(const std::string& path);
			void unload(GLuint shader);
	};

	using Shader = ShaderManager::Resource;
}
