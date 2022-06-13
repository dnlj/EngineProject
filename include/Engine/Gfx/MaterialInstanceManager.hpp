#pragma once

// Engine
#include <Engine/ResourceManager.hpp>
#include <Engine/Gfx/Material.hpp>


namespace Engine::Gfx {
	class MaterialInstanceManager : public ResourceManager<MaterialInstance> {
		using ResourceManager::ResourceManager;
	};
}
