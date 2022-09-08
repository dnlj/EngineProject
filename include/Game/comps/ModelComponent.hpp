#pragma once

// Engine
#include <Engine/Gfx/Model.hpp>


namespace Game {
	class ModelComponent {
		public:
			// TODO: engine::gfx::model
			// TODO: makes sense for the armature to be a separate comp
			// ^^^^: How to org? Currently our instances have a nodeId. how does that relate between components.
			std::vector<Engine::Gfx::MeshInstance> meshes;
	};
}
