#pragma once

// Engine
#include <Engine/ResourceRef.hpp>
#include <Engine/Gfx/TextureType.hpp>


namespace Engine::Gfx {
	using ShaderRef = ResourceRef<class Shader>;
	using BufferRef = ResourceRef<class Buffer>;
	using MeshRef = ResourceRef<class Mesh2>;
	using MaterialRef = ResourceRef<class Material>;
	using MaterialInstanceRef = ResourceRef<class MaterialInstance>;
	using VertexAttributeLayoutRef = ResourceRef<class VertexAttributeLayout>;

	template<class> class TextureInfo;
	template<int32, TextureType> class Texture;

	using TextureGeneric = Texture<0, TextureType::Unknown>;
	using Texture1D = Texture<1, TextureType::Target1D>;
	using Texture2D = Texture<2, TextureType::Target2D>;
	using Texture3D = Texture<3, TextureType::Target3D>;
	using Texture1DArray = Texture<2, TextureType::Target1DArray>;
	using Texture2DArray = Texture<3, TextureType::Target2DArray>;

	using TextureGenericRef = ResourceRef<TextureInfo<TextureGeneric>>;
	using Texture1DRef = ResourceRef<TextureInfo<Texture1D>>;
	using Texture2DRef = ResourceRef<TextureInfo<Texture2D>>;
	using Texture3DRef = ResourceRef<TextureInfo<Texture3D>>;
	using Texture1DArrayRef = ResourceRef<TextureInfo<Texture1DArray>>;
	using Texture2DArrayRef = ResourceRef<TextureInfo<Texture2DArray>>;
	// TODO: cubemap
	// TODO: cubemap array
}
