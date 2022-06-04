#pragma once

// Engine
#include <Engine/ResourceManager.hpp>
#include <Engine/Gfx/Shader.hpp>


namespace Engine::Gfx {
	class ShaderManager : public ResourceManager<Shader> {
		using ResourceManager::ResourceManager;
	};

	using ShaderRef = ShaderManager::ResourceRef;

	class ShaderLoader final : public ResourceLoader<std::string, Shader> {
		using ResourceLoader::ResourceLoader;
		virtual Resource load(const Key& key) override { return key; }
	};
}
