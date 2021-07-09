// Harfbuzz
#include <hb-ft.h>

// Engine
#include <Engine/Gui/FontManager.hpp>


namespace Engine::Gui {
	FontManager::FontManager() {
		if (const auto err = FT_Init_FreeType(&ftlib)) {
			ENGINE_ERROR("FreeType error: ", err); // TODO: actual error
		}

		if (const auto err = FT_New_Face(ftlib, "assets/arial.ttf", 0, &face)) {
			ENGINE_ERROR("FreeType error: ", err); // TODO: actual error
		}

		if (const auto err = FT_Set_Pixel_Sizes(face, 0, 32)) {
			ENGINE_ERROR("FreeType error: ", err); // TODO: actual error
		}

		font = hb_ft_font_create_referenced(face);
	}

	FontManager::~FontManager() {
		hb_font_destroy(font);

		if (const auto err = FT_Done_Face(face)) {
			ENGINE_ERROR("FreeType error: ", err); // TODO: actual error
		}
			
		if (const auto err = FT_Done_FreeType(ftlib)) {
			ENGINE_ERROR("FreeType error: ", err); // TODO: actual error
		}
	}
}
