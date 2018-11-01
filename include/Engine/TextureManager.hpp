#pragma once

// STD
#include <unordered_map>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// Engine
#include <Engine/ResourceManager.hpp>

// TODO: Document
namespace Engine {
	class TextureManager : public Engine::ResourceManager<TextureManager, GLuint> {
		friend class Engine::ResourceManager<TextureManager, GLuint>;
		private:
			GLuint load(const std::string& path);
			void unload(GLuint texture);
	};

	using Texture = TextureManager::ResourceType;
}
