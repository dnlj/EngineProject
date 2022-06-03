#pragma once

// STD
#include <unordered_map>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// Engine
#include <Engine/ResourceManager.hpp>
#include <Engine/Gfx/Shader.hpp>

// TODO: Document
namespace Engine {
	// TODO: rm
	class ShaderManager : public Engine::ResourceManager<ShaderManager, Gfx::Shader> {
		friend class Engine::ResourceManager<ShaderManager, Gfx::Shader>;
		private:
			StorePtr load(const std::string& path) { return std::make_unique<Gfx::Shader>(path); };
			void unload(Gfx::Shader shader);
	};

	// TODO: rm
	using ShaderRef = ShaderManager::Resource;
}
