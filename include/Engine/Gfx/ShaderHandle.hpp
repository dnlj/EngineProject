#pragma once

// Engine
#include <Engine/engine.hpp>


namespace Engine::Gfx {
	/**
	 * A non-owning reference to a Shader.
	 * @see Shader
	 */
	class ShaderHandle {
		private:
			uint32 handle = 0;

		public:
			ShaderHandle() = default;
			ENGINE_INLINE explicit ShaderHandle(uint32 h) noexcept : handle{h} {}
			ENGINE_INLINE auto get() const noexcept { return handle; }
			ENGINE_INLINE auto operator<=>(const ShaderHandle&) const noexcept = default;
	};
}
