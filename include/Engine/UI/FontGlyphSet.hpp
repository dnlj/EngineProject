#pragma once

// STD
#include <vector>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// FreeType
#include <ft2build.h>
#include <freetype/freetype.h>

// Harfbuzz
#include <hb.h>

// Engine
#include <Engine/FlatHashMap.hpp>
#include <Engine/Gfx/Texture.hpp>
#include <Engine/UI/Bounds.hpp>
#include <Engine/UI/FreeType.hpp>


namespace Engine::UI {
	using Font = class FontGlyphSet*;

	class ShapeGlyph {
		public:
			uint32 index;
			uint32 cluster;
			glm::vec2 offset;
			glm::vec2 advance;
	};
}

namespace Engine::UI {
	/**
	 * Supports a number of operations needed to render strings in a given font face and size on the gpu.
	 *
	 * - Loading glyphs into a texture.
	 * - Providing metrics for glyph layout.
	 * - Providing a subset of metrics on the gpu.
	 * - Converting from strings to a positioned glyph sequence.
	 * 
	 */
	class FontGlyphSet {
		private:
			constexpr static float32 mscale = 1.0f / 64; // We can also just do `metric >> 6` if we dont care about float/rounding

			struct GlyphData {
				// Make sure to consider GLSL alignment rules
				glm::vec2 size; // Size in texels
				float32 _size_padding[2];

				glm::vec3 offset; // Offset in texels
				float32 _offset_padding[1];
			}; static_assert(sizeof(GlyphData) == sizeof(float32) * 8);

			struct GlyphMetrics {
				glm::vec2 bearing;
				uint32 index;
			};

		private:
			FT_Face ftFace;
			FT_Size ftSize;
			hb_font_t* hbFont;

			FlatHashMap<uint32, uint32> glyphIndexToLoadedIndex;
			std::vector<GlyphData> glyphData;
			std::vector<GlyphMetrics> glyphMetrics;

			Gfx::Texture2D glyphTex;
			int32 glyphTexSize = 0;
			glm::vec2 maxGlyphSize;
			glm::ivec2 indexBounds;
			int nextGlyphIndex = 0; // TODO: glyph index recycling

		public:
			FontGlyphSet();
			FontGlyphSet(const FontGlyphSet&) = delete;
			FontGlyphSet(FontGlyphSet&&) = delete;
			~FontGlyphSet();

			void init(FT_Face face, int32 size);

			void loadGlyph(const uint32 index);

			ENGINE_INLINE int32 getGlyphTextureSize() const noexcept { return glyphTexSize; }

			ENGINE_INLINE bool isGlyphLoaded(const uint32 index) const noexcept { return glyphIndexToLoadedIndex.contains(index); };

			ENGINE_INLINE const auto& getGlyphTexture() const noexcept { return glyphTex; }

			ENGINE_INLINE const auto& _debug_getGlyphData() const noexcept { return glyphData; } // TODO: just merge glyphData and metrics? or what?

			ENGINE_INLINE auto getGlyphIndex(uint32 glyph) { return glyphIndexToLoadedIndex[glyph]; }

			ENGINE_INLINE auto& getManager() const noexcept {
				ENGINE_DEBUG_ASSERT(ftFace->generic.data, "No font manager assigned to glyph set. This is a bug.");
				return *reinterpret_cast<class FontManager*>(ftFace->generic.data);
			}

			void updateDataBuffer();

			/**
			 * Populate a ShapedString with glyph position and bounds info.
			 */
			void shapeString(class ShapedString& str); // TODO: rm - just use other overload

			ENGINE_INLINE void shapeString(std::string_view str, std::vector<ShapeGlyph>& glyphs, Bounds& bounds) { shapeStringImpl(str, glyphs, bounds); }
			ENGINE_INLINE void shapeString(std::string_view str, RingBuffer<ShapeGlyph>& glyphs, Bounds& bounds) { shapeStringImpl(str, glyphs, bounds); }

			/**
			 * Gets the font specified line height (i.e. the recommended distance between baselines)
			 * 
			 * This is different than the CSS line-height property which ignores
			 * the font specified height and simply applies a multiplier of the
			 * font size or a fixed px value.
			 *
			 * Example:
			 * A 32px Arial font has a specified height of 37px, but CSS ignores this
			 * value completely and with a line-height of 1.0 would return a line
			 * height of 32px not 37px.
			 */
			ENGINE_INLINE auto getLineHeight() const noexcept { return ftSize->metrics.height * mscale; }

			ENGINE_INLINE auto getAscent() const noexcept {
				// Can not use `ftSize->metrics.ascender` because it may be off by multiple pixels.
				// See https://freetype.org/freetype2/docs/reference/ft2-base_interface.html#ft_size_metrics
				return float32(FreeType::roundF26d6(FT_MulFix(ftFace->ascender, ftSize->metrics.y_scale)) >> 6);
			}

			ENGINE_INLINE auto getDescent() const noexcept {
				// Can not use `ftSize->metrics.descender` because it may be off by multiple pixels.
				// See https://freetype.org/freetype2/docs/reference/ft2-base_interface.html#ft_size_metrics
				return float32(FreeType::roundF26d6(FT_MulFix(ftFace->descender, ftSize->metrics.y_scale)) >> 6);
			}

			/**
			 * Gets the distance from the top of the highest letter to the bottom of the lowest.
			 * This is as specified by the font. It is possible that some glyphs will overshoot.
			 */
			ENGINE_INLINE auto getBodyHeight() const noexcept { return getAscent() - getDescent(); }

		private:
			template<class ShapeGlyphCont>
			void shapeStringImpl(std::string_view str, ShapeGlyphCont& glyphs, Bounds& bounds);

			void initMaxGlyphSize();
	};
}
