#pragma once

// FreeType
#include <ft2build.h>
#include <freetype/freetype.h>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Gui/Font.hpp>


namespace Engine::Gui {
	class FontManager {
		public: // TODO: private
			FT_Library ftlib;
			Font font;

		public:
			FontManager();
			~FontManager();
	};
}
