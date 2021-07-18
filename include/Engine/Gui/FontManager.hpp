#pragma once

// FreeType
#include <ft2build.h>
#include <freetype/freetype.h>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Gui/FontGlyphSet.hpp>
#include <Engine/Gui/FontId.hpp>


template<> 
struct Engine::Hash<Engine::Gui::FontId> {
	size_t operator()(const Engine::Gui::FontId& val) const {
		static_assert(sizeof(size_t) == sizeof(val.font));
		size_t seed = reinterpret_cast<const size_t&>(val.font);
		hashCombine(seed, val.size);
		return seed;
	}
};

namespace Engine::Gui {
	class FontManager {
		private:
			FT_Library ftlib;
			hb_buffer_t* workingBuffer;
			FlatHashMap<std::string, FT_Face> pathToFace;
			FlatHashMap<FontId, std::unique_ptr<FontGlyphSet>> fontIdToGlyphSet;

		public:
			FontManager();
			~FontManager();

			FontId createFont(const std::string& path, int32 size);

			ENGINE_INLINE auto* getFontGlyphSet(FontId fid) {
				auto found = fontIdToGlyphSet.find(fid);
				return found == fontIdToGlyphSet.end() ? nullptr : found->second.get();
			}

			ENGINE_INLINE const auto* getFontGlyphSet(FontId fid) const {
				return const_cast<FontManager*>(this)->getFontGlyphSet(fid);
			}

			void shapeString(ShapedString& str, FontGlyphSet* glyphSet);
	};
}

