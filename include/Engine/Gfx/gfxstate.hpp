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

			// TOOD: still need to store specific inputs "material instance" that fit the shape (textures, uniforms, ubo, ssbo, etc.)

		public:
	};
	class MaterialRef {};

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
