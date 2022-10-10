#pragma once

// Engine
#include <Engine/Gfx/Mesh.hpp>


namespace Engine::Gfx {
	class MeshManager : public ResourceManager<Mesh> {
		using ResourceManager::ResourceManager;
	};
}
