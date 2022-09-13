#pragma once

// Engine
#include <Engine/Gfx/resources.hpp>


namespace Engine::Gfx {
	class MeshNode {
		public:
			MeshRef mesh;
			MaterialInstanceRef mat;
			NodeId nodeId;
	};
}
