// STD
#include <algorithm>

// FreeType
#include <freetype/ftsizes.h>

// Harfbuzz
#include <hb-ft.h>

// Engine
#include <Engine/UI/FontManager.hpp>
#include <Engine/UI/FontGlyphSet.hpp>
#include <Engine/UI/common.hpp>


namespace Engine::UI {
	FontManager::FontManager() {
		if (const auto err = FT_Init_FreeType(&ftlib)) {
			ENGINE_ERROR("FreeType error: ", getFreeTypeErrorString(err));
		}

		workingBuffer = hb_buffer_create();
		//hb_buffer_set_cluster_level(workingBuffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES);
		hb_buffer_set_cluster_level(workingBuffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);
		//hb_buffer_set_cluster_level(workingBuffer, HB_BUFFER_CLUSTER_LEVEL_CHARACTERS);
	}

	FontManager::~FontManager() {
		// Need to destroy dependants before lib
		fontIdToGlyphSet.clear();

		if (const auto err = FT_Done_FreeType(ftlib)) {
			ENGINE_ERROR("FreeType error: ", getFreeTypeErrorString(err));
		}

		hb_buffer_destroy(workingBuffer);
	}

	auto FontManager::createFont(const std::string& path, int32 size) -> Font {
		// TODO: we realy would want to check against max texture size or similar.
		if (size < 1 || size > 0xFFFF) [[unlikely]] {
			ENGINE_WARN("Invalid font size ", size, ". Defaulting to 32");
			size = 32;
		}

		FT_Face face;
		// TODO: look into is_transparent and heterogeneous lookup. ideally we would just take a string_view
		if (auto found = pathToFace.find(path); found == pathToFace.end()) {
			auto [it, _] = pathToFace.insert({path, nullptr});
			found = it;

			// Setup FreeType face
			if (const auto err = FT_New_Face(ftlib, path.data(), 0, &found->second)) {
				ENGINE_ERROR("FreeType error: ", getFreeTypeErrorString(err));
			}

			FT_Done_Size(found->second->size);
			face = found->second;
			face->generic.data = this;

			// Setup HarfBuzz face
			// For the time being this isnt viable. See FontGlyphSet::init for details.
			//hb_face_t* hbFace = hb_ft_face_create(face, nullptr);
			//face->generic.data = hbFace;
			//face->generic.finalizer = [](void* obj){
			//	auto face = reinterpret_cast<FT_Face>(obj);
			//	hb_face_destroy(reinterpret_cast<hb_face_t*>(face->generic.data));
			//};
		} else {
			face = found->second;
		}

		FontId id = {face, size};

		auto found = fontIdToGlyphSet.find(id);
		if (found == fontIdToGlyphSet.end()) {
			auto [it, _] = fontIdToGlyphSet.insert({id, std::make_unique<FontGlyphSet>()});
			found = it;
			found->second->init(face, size);
		}

		return found->second.get();
	}
}
