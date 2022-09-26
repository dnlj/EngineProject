#pragma once

// Engine
#include <Engine/ResourceRef.hpp>
#include <Engine/Gfx/TextureInfo.hpp>


namespace Engine::Gfx {
	using NodeId = int32;
	using BoneId = int32;

	using ShaderRef = ResourceRef<class Shader>;
	using BufferRef = ResourceRef<class Buffer>;
	using MeshRef = ResourceRef<class Mesh2>;
	using MaterialRef = ResourceRef<class Material>;
	using MaterialInstanceRef = ResourceRef<class MaterialInstance>;
	using VertexAttributeLayoutRef = ResourceRef<class VertexAttributeLayout>;
	using AnimationRef = ResourceRef<class Animation>;

	template<int32, TextureType> class Texture;
	using TextureGeneric = Texture<0, TextureType::Unknown>;
	using Texture1D = Texture<1, TextureType::Target1D>;
	using Texture2D = Texture<2, TextureType::Target2D>;
	using Texture3D = Texture<3, TextureType::Target3D>;
	using Texture1DArray = Texture<2, TextureType::Target1DArray>;
	using Texture2DArray = Texture<3, TextureType::Target2DArray>;

	using TextureGenericInfo = TextureInfo<TextureGeneric>;
	using Texture1DInfo = TextureInfo<Texture1D>;
	using Texture2DInfo = TextureInfo<Texture2D>;
	using Texture3DInfo = TextureInfo<Texture3D>;
	using Texture1DArrayInfo = TextureInfo<Texture1DArray>;
	using Texture2DArrayInfo = TextureInfo<Texture2DArray>;
}

namespace Engine {
	template<> class ResourceRef<Gfx::TextureGenericInfo>;
}

namespace Engine::Gfx {
	using TextureGenericRef = ResourceRef<TextureGenericInfo>;
	using TextureRef = TextureGenericRef;
	using Texture1DRef = ResourceRef<Texture1DInfo>;
	using Texture2DRef = ResourceRef<Texture2DInfo>;
	using Texture3DRef = ResourceRef<Texture3DInfo>;
	using Texture1DArrayRef = ResourceRef<Texture1DArrayInfo>;
	using Texture2DArrayRef = ResourceRef<Texture2DArrayInfo>;
	// TODO: cubemap
	// TODO: cubemap array
}
