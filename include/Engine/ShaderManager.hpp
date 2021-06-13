#pragma once

// STD
#include <unordered_map>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// Engine
#include <Engine/ResourceManager.hpp>
#include <Engine/Graphics/Shader.hpp>

// TODO: Document
namespace Engine {
	class ShaderManager : public Engine::ResourceManager<ShaderManager, Graphics::Shader> {
		friend class Engine::ResourceManager<ShaderManager, Graphics::Shader>;
		private:
			StorePtr load(const std::string& path) { return std::make_unique<Graphics::Shader>(path); };
			void unload(Graphics::Shader shader);
	};

	using ShaderRef = ShaderManager::Resource;
}
