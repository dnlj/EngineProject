// STD
#include <algorithm>

// FreeType
#include <freetype/ftsizes.h>

// Harfbuzz
#include <hb-ft.h>

// Engine
#include <Engine/Gui/FontManager.hpp>


namespace Engine::Gui {
	FontManager::FontManager() {
		if (const auto err = FT_Init_FreeType(&ftlib)) {
			ENGINE_ERROR("FreeType error: ", err); // TODO: actual error
		}

		workingBuffer = hb_buffer_create();
	}

	FontManager::~FontManager() {
		if (const auto err = FT_Done_FreeType(ftlib)) {
			ENGINE_ERROR("FreeType error: ", err); // TODO: actual error
		}

		hb_buffer_destroy(workingBuffer);
	}

	FontId FontManager::createFont(const std::string& path, int32 size) {
		// TODO: we realy would want to check against max texture size or similar.
		if (size < 1 || size > 0xFFFF) [[unlikely]] {
			ENGINE_WARN("Invalid font size ", size, ". Defaulting to 32");
			size = 32;
		}

		FT_Face face;
		{
			auto found = pathToFace.find(path);// TODO: look into is_transparent and heterogeneous lookup. ideally we would just take a string_view
			if (found == pathToFace.end()) {
				auto [it, _] = pathToFace.insert({path, nullptr});
				found = it;

				if (const auto err = FT_New_Face(ftlib, path.data(), 0, &found->second)) {
					ENGINE_ERROR("FreeType error: ", err); // TODO: actual error
				}

				FT_Done_Size(found->second->size);
			}

			face = found->second;
		}

		FontId id = {face, size};
		{
			auto found = fontIdToGlyphSet.find(id);
			if (found == fontIdToGlyphSet.end()) {
				auto [it, _] = fontIdToGlyphSet.insert({id, std::make_unique<FontGlyphSet>()});
				found = it;
				found->second->init(face, size, workingBuffer);
			}
		}

		return id;
	}
}
