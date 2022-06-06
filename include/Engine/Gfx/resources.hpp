#pragma once

// Engine
#include <Engine/ResourceRef.hpp>


namespace Engine::Gfx {
	using ShaderRef = ResourceRef<class Shader>;
	using BufferRef = ResourceRef<class Buffer>;
	using MeshRef = ResourceRef<class Mesh2>;
	using TextureRef = ResourceRef<class TextureInfo>;
	using VertexAttributeLayoutRef = ResourceRef<class VertexAttributeLayout>;
}
