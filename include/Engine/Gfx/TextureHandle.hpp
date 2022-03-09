#pragma once

// Engine
#include <Engine/engine.hpp>


namespace Engine {
	template<int32 D, uint32 Target>
	class TextureHandle {
		private:
			uint32 handle = 0;

		public:
			TextureHandle() = default;
			ENGINE_INLINE explicit TextureHandle(uint32 h) noexcept : handle{h} {}
			ENGINE_INLINE auto get() const noexcept { return handle; }
			ENGINE_INLINE auto operator<=>(const TextureHandle&) const noexcept = default;
	};

	using TextureHandle1D = TextureHandle<1, GL_TEXTURE_1D>;
	using TextureHandle2D = TextureHandle<2, GL_TEXTURE_2D>;
	using TextureHandle3D = TextureHandle<3, GL_TEXTURE_3D>;
	using TextureHandle1DArray = TextureHandle<2, GL_TEXTURE_1D_ARRAY>;
	using TextureHandle2DArray = TextureHandle<3, GL_TEXTURE_2D_ARRAY>;
}
