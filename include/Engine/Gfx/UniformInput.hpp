#pragma once


namespace Engine::Gfx {
	// Might also contain things like UBO and SSBO
	enum class UniformInput : uint32 {
		None				= 0,
		ModelViewProj		= 1 << 0,
		ModelViewProjArray	= 1 << 1,
		Armature			= 1 << 2,
	};
}
