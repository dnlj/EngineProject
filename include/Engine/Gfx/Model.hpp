#pragma once

// Engine
#include <Engine/Gfx/Armature.hpp>
#include <Engine/Gfx/resources.hpp>


namespace Engine::Gfx {
	class MeshInstance { // TODO: should be part of Engine?
		public:
			NodeId nodeId; // TODO: should this be part of a wrapped type? doesnt really belong
			MeshRef mesh;
			MaterialInstanceRef material;

			glm::mat4 mvp; // TODO: find better solution for uniforms

			// TODO: Textures(TextureRef)
			// TODO: Buffers(BufferRef)
			// TODO: Uniforms(???)
	};

	//class Model {
	//	private:
	//		std::vector<MeshInstance> meshes;
	//
	//	public:
	//};
}
