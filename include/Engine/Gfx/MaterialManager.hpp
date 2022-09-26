#pragma once

// Engine
#include <Engine/ResourceManager.hpp>
#include <Engine/Gfx/Material.hpp>


namespace Engine::Gfx {
	class MaterialManager : public ResourceManager<Material> {
		using ResourceManager::ResourceManager;
	};

	class MaterialLoader final : public ResourceLoader<ShaderRef, Material> {
		using ResourceLoader::ResourceLoader;
		virtual Resource load(const Key& key) override {
			return key;
		}
	};
}
