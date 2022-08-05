#pragma once

// Engine
#include <Engine/engine.hpp>
#include <Engine/Gfx/TextureType.hpp>


namespace Engine::Gfx {
	/**
	 * A non-owning reference to a Texture.
	 * @see Texture
	 */
	template<int32 D, TextureType Target>
	class TextureHandle {
		private:
			uint32 handle = 0;

		public:
			TextureHandle() = default;
			ENGINE_INLINE explicit TextureHandle(uint32 h) noexcept : handle{h} {}
			ENGINE_INLINE auto get() const noexcept { return handle; }
			ENGINE_INLINE auto operator<=>(const TextureHandle&) const noexcept = default;
	};

	using TextureHandleGeneric = TextureHandle<0, TextureType::Unknown>;
	using TextureHandle1D = TextureHandle<1, TextureType::Target1D>;
	using TextureHandle2D = TextureHandle<2, TextureType::Target2D>;
	using TextureHandle3D = TextureHandle<3, TextureType::Target3D>;
	using TextureHandle1DArray = TextureHandle<2, TextureType::Target1DArray>;
	using TextureHandle2DArray = TextureHandle<3, TextureType::Target2DArray>;
}
