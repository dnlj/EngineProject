#pragma once

// STD
#include <concepts>

// Engine
#include <Engine/Gfx/Shader.hpp>
#include <Engine/ArrayView.hpp>
#include <Engine/Gfx/NumberType.hpp>
#include <Engine/Gfx/VertexInput.hpp>
#include <Engine/Gfx/UniformInput.hpp>
#include <Engine/Gfx/VertexAttributeLayout.hpp>


// TODO: split/sort/organaize everything in this file.

namespace Engine::Gfx {
	// CPU
	//class Armature {}; // TODO: this isnt nessarily an armature, more of a embedded scene graph.
	//class Animation {};

	class ShaderInfo { // TODO: bad name, more descriptive
		Shader shader;
		VertexInput vertexInputs;
		UniformInput shaderInputs;
	};
	class ShaderInfoRef {};

	// Groups a specific shader with inputs for that shader
	class Material {
		private:
			ShaderInfoRef shader;

			// TOOD: still need to store specific inputs that fit the shape (textures, uniforms, ubo, ssbo, etc.)

		public:
	};
	class MaterialRef {};

	class Buffer_ChangeToNewBuffer {};

	class ModelComponent {
		//std::vector<MeshRef> meshes;
	};

	class Armature2 {};
	class ArmatureComponent {
		Armature2 arm;
	};

	class AnimationComponent {
		// TODO: not sure where to store animations
	};

	class EntityRenderable { // TODO: this would be an ecs entity with these components
		private:
			ArmatureComponent armComp;
			ModelComponent modelComp;
	};

	// I think we really need more specialized batches unless we have a good way to rep uniform data
	struct DrawBatch {
		uint32 program;
		uint32 vao;

		struct TextureBinding {
			uint32 texture;
			uint32 binding;
		} textures[4];

		struct BufferBinding {
			uint32 type; // UBO, SSBO. See glBindBufferRange
			uint32 buffer;
			uint32 binding;
		} buffers[4];

		struct UniformData {
			// TODO: ????
		} uniforms[1];

	};

	/*
		.
		build batches

		loop all:
			program
			? texture bindings (not texture uniforms) ?
			VAO
			vbo
			buffers bindings (ubo, ssbo, etc)
			uniform
		
		glMultiDrawElementsIndirect(draw.vertexType, draw.indexType, draw.offset, draw.count, 0);

	*/
}
