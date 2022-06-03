#pragma once

// Engine
#include <Engine/ResourceManager2.hpp>
#include <Engine/Gfx/Shader.hpp>


namespace Engine::Gfx {
	class ShaderManager : public ResourceManager2<Shader> {
		using ResourceManager2::ResourceManager2;
	};

	using ShaderRef = ShaderManager::ResourceRef;

	class ShaderLoader final : public ResourceLoader<std::string, Shader> {
		using ResourceLoader::ResourceLoader;
		virtual Resource load(const Key& key) override { return key; }
	};
}
