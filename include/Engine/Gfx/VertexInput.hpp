#pragma once


namespace Engine::Gfx {
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
		DrawId		= 1 << 7,
	};
}
