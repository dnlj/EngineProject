// STD
#include <algorithm>

// Harfbuzz
#include <hb-ft.h>

// Engine
#include <Engine/Gui/FontManager.hpp>


namespace Engine::Gui {
	FontManager::FontManager() {
		if (const auto err = FT_Init_FreeType(&ftlib)) {
			ENGINE_ERROR("FreeType error: ", err); // TODO: actual error
		}

		font.init(ftlib);
	}

	FontManager::~FontManager() {
		if (const auto err = FT_Done_FreeType(ftlib)) {
			ENGINE_ERROR("FreeType error: ", err); // TODO: actual error
		}
	}
}
