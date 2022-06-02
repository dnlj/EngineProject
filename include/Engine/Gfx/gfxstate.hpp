#pragma once

// STD
#include <concepts>

// Engine
#include <Engine/Gfx/Shader.hpp>
#include <Engine/ArrayView.hpp>
#include <Engine/Gfx/NumberType.hpp>


// TODO: split/sort/organaize everything in this file.

namespace Engine::Gfx {
	// CPU
	//class Armature {}; // TODO: this isnt nessarily an armature, more of a embedded scene graph.
	//class Animation {};

	// GPU
	class MeshRef {};

	// TODO: should this be called VertexAttribute instead?
	enum class VertexInput : uint32 {
		None		= 0,
		Position	= 1 << 0,
		TexCoord	= 1 << 1,
		Color		= 1 << 2,
		Normal		= 1 << 3,
		Tangent		= 1 << 4,
		BoneIndices = 1 << 5,
		BoneWeights	= 1 << 6,
	};

	// Might also contain things like UBO and SSBO
	enum class UniformInput : uint32 {
		None				= 0,
		ModelViewProj		= 1 << 0,
		ModelViewProjArray	= 1 << 1,
		Armature			= 1 << 2,
	};

	// Describes the location of a specific attribute within a buffer
	class VertexAttributeDesc {
		public:
			VertexAttributeDesc() = default;
			VertexAttributeDesc(VertexInput input, uint16 size, NumberType type, uint32 offset, bool normalize)
				: input{input}
				, type{type}
				, offset{offset}
				, size{size}
				, normalize{normalize} {
			}

			VertexInput input = {};
			NumberType type = {};
			uint32 offset = {};
			uint16 size = {};
			bool normalize = {};
			// bool normalize : 1;
			// uint16 binding : 15; - may want binding later if we want to support multiple
			bool operator==(const VertexAttributeDesc&) const = default;
	}; static_assert(sizeof(VertexAttributeDesc) == 16);

	class VertexAttributeDescList {
		private:
			constexpr static size_t Count = 16;
			VertexAttributeDesc attribs[Count]; // TODO: make VertexAttributeDescList always have sorted attribs

		public:
			template<size_t N>
			VertexAttributeDescList(const VertexAttributeDesc (&init)[N]) {
				static_assert(N <= Count, "To many vertex attributes");
				std::copy(init, init+N, attribs);
			}

			VertexAttributeDescList(const VertexAttributeDesc* data, size_t count) {
				ENGINE_ASSERT_WARN(count <= Count, "To many vertex attributes");
				std::copy(data, data + std::min(count, Count), attribs);
			}

			ENGINE_INLINE constexpr auto size() const { return Count; }
			ENGINE_INLINE const auto begin() const { return attribs; }
			ENGINE_INLINE const auto end() const { return attribs + Count; }
			ENGINE_INLINE const auto cbegin() const { return begin(); }
			ENGINE_INLINE const auto cend() const { return end(); }
			const VertexAttributeDesc& operator[](uint32 i) const { return attribs[i]; }
			bool operator==(const VertexAttributeDescList&) const = default;
	};

	// Maps a buffer layout to a vertex input shape (built from a VertexAttributeDesc[])
	class VertexAttributeLayout {
		public:
			// TODO: should this also have a vertexInputs? i dont think we actually need it though.
			uint32 vao;
	};

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
	// A single set of vertices with a single material.
	class Mesh2 {
		/* Vertex layouts could actually draw from multiple buffers (binding points), but atm we never actually do that.
		 * @see glVertexArrayVertexBuffer
		 * @see glVertexArrayAttribBinding
		 */
		Buffer_ChangeToNewBuffer vbuff;
		MaterialRef mat;
		//VertexAttributeLayoutRef layout;
		// TODO: still need to setup buffers - glVertexArrayVertexBuffer(s)

		Buffer_ChangeToNewBuffer ibuff;
		uint32 offset = 0;
		uint32 count = 0;

		// Not needed currently because we only use one vertex buffer.
		// If in the future we need to draw from multiple vertex buffers (binding points) we will need to be able to describe buffer -> binding point mappings
		// vector<pair<int, int>> bufferToBindingMapping;
	};
	class Mesh2Ref {};

	class ModelComponent {
		std::vector<Mesh2Ref> meshes;
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

template<>
struct Engine::Hash<Engine::Gfx::VertexAttributeDescList> {
	size_t operator()(const Engine::Gfx::VertexAttributeDescList& val) const {
		return hashBytes(val.begin(), val.size() * sizeof(val[0]));
	}
};
