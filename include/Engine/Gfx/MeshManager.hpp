#pragma once

// Engine
#include <Engine/Gfx/Mesh2.hpp>


namespace Engine::Gfx {
	class MeshManager : public ResourceManager<Mesh2> {
		using ResourceManager::ResourceManager;
	};
}
