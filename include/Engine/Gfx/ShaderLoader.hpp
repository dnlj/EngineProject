#pragma once

// Engine
#include <Engine/ResourceManager2.hpp>
#include <Engine/Gfx/Shader.hpp>


namespace Engine::Gfx {
	// TODO: rename
	class ShaderManager2 : public ResourceManager2<Shader> {
		using ResourceManager2::ResourceManager2;
	};

	using ShaderRef2 = ShaderManager2::ResourceRef;

	class ShaderLoader final : public ResourceLoader<std::string, Shader> {
		using ResourceLoader::ResourceLoader;
		virtual Resource load(const Key& key) override { return key; }
	};
}
