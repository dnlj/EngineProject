#pragma once

// Engine
#include <Engine/ResourceManager.hpp>
#include <Engine/Gfx/Material.hpp>


namespace Engine::Gfx {
	class MaterialManager : public ResourceManager<Material> {
		using ResourceManager::ResourceManager;
	};
}
