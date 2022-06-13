#pragma once

// Engine
#include <Engine/Gfx/resources.hpp>
#include <Engine/Gfx/Material.hpp>


namespace Game {
	class MeshInstance { // TODO: should be part of Engine?
		public:
			Engine::Gfx::Material material;
			Engine::Gfx::MeshRef mesh;

			glm::mat4 mvp; // TODO: find better solution for uniforms

			// TODO: move into MaterialInstance
			Engine::Gfx::MaterialParams params;

			// TODO: Textures(TextureRef)
			// TODO: Buffers(BufferRef)
			// TODO: Uniforms(???)
	};

	class ModelComponent {
		public:
			// TODO: makes sense for the armature to be a separate comp
			// TODO: cont. How to org? Currently our instances have a nodeId. how does that relate between components.
			std::vector<MeshInstance> meshes;
	};
}
