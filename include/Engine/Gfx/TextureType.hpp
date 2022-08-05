#pragma once


namespace Engine::Gfx {
	enum class TextureType {
		Unknown,
		Target1D = GL_TEXTURE_1D,
		Target2D = GL_TEXTURE_2D,
		Target3D = GL_TEXTURE_3D,
		Target1DArray = GL_TEXTURE_1D_ARRAY,
		Target2DArray = GL_TEXTURE_2D_ARRAY,
	};

	ENGINE_INLINE constexpr auto toGL(TextureType type) noexcept {
		return static_cast<GLenum>(type);
	}
}
