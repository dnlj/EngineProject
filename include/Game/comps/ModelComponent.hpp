#pragma once

// Engine
#include <Engine/Gfx/resources.hpp>


namespace Game {
	class ModelComponent {
		public:
			// TODO: makes sense for the armature to be a separate comp
			// TODO: cont. How to org? Currently our instances have a nodeId. how does that relate between components.
			Engine::Gfx::ShaderRef shader;
			Engine::Gfx::MeshRef mesh;



			// TODO: Textures(TextureRef)
			// TODO: Buffers(BufferRef)
			// TODO: Uniforms(???)
	};
}
