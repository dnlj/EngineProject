#pragma once

// Engine
#include <Engine/UI/Panel.hpp>


namespace Engine::UI {
	class ImageDisplay : public Panel {
		private:
			TextureHandle2D tex;

		public:
			using Panel::Panel;
			ENGINE_INLINE void setTexture(TextureHandle2D texture) noexcept { tex = texture; }
			void render() override { ctx->drawTexture(tex, {}, getSize()); }
	};
}
