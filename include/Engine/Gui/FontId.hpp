#pragma once

// Engine
#include <Engine/Engine.hpp>


namespace Engine::Gui {
	struct FontId {
		FT_Face font;
		int32 size;

		ENGINE_INLINE bool operator==(const FontId& other) const noexcept {
			return font == other.font && size == other.size;
		}
	};
}
