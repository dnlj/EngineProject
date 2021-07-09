#pragma once

// Engine
#include <Engine/Engine.hpp>

// FreeType
#include <ft2build.h>
#include <freetype/freetype.h>

// Harfbuzz
#include <hb.h>


namespace Engine::Gui {
	class FontManager {
		public: // TODO: private
			FT_Library ftlib;

			FT_Face face;
			hb_font_t* font;

		public:
			FontManager();
			~FontManager();
	};
}
