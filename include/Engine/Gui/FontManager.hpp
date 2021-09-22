#pragma once

// FreeType
#include <ft2build.h>
#include <freetype/freetype.h>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/FlatHashMap.hpp>


struct hb_buffer_t;


namespace Engine::Gui::Detail {
	struct FontId {
		FT_Face font;
		int32 size;

		ENGINE_INLINE bool operator==(const FontId& other) const noexcept {
			return font == other.font && size == other.size;
		}
	};
}

template<> 
struct Engine::Hash<Engine::Gui::Detail::FontId> {
	size_t operator()(const Engine::Gui::Detail::FontId& val) const {
		static_assert(sizeof(size_t) == sizeof(val.font));
		size_t seed = reinterpret_cast<const size_t&>(val.font);
		hashCombine(seed, val.size);
		return seed;
	}
};

namespace Engine::Gui {
	using Font = class FontGlyphSet*;

	class FontManager {
		private:
			using FontId = Detail::FontId;

		private:
			FT_Library ftlib;
			hb_buffer_t* workingBuffer;
			FlatHashMap<std::string, FT_Face> pathToFace;
			FlatHashMap<FontId, std::unique_ptr<FontGlyphSet>> fontIdToGlyphSet;

		public:
			FontManager();
			~FontManager();

			Font createFont(const std::string& path, int32 size);

			ENGINE_INLINE auto getWorkingBuffer() const noexcept { return workingBuffer; }

			void updateAllFontDataBuffers();
	};
}

