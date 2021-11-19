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
#include <Engine/Gui/Bounds.hpp>


namespace Engine::Gui {
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
			constexpr static float32 mscale = 1.0f / 64;

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

			GLuint glyphSSBO = 0;
			GLsizei glyphSSBOSize = 0;
			FlatHashMap<uint32, uint32> glyphIndexToLoadedIndex;
			std::vector<GlyphData> glyphData;
			std::vector<GlyphMetrics> glyphMetrics;
			
			Texture2D glyphTex;
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

			ENGINE_INLINE bool isGlyphLoaded(const uint32 index) const noexcept { return glyphIndexToLoadedIndex.contains(index); };

			ENGINE_INLINE void ensureGlyphLoaded(const uint32 index) { if (!isGlyphLoaded(index)) { loadGlyph(index); } }

			ENGINE_INLINE const auto& getGlyphTexture() const noexcept { return glyphTex; }

			ENGINE_INLINE const auto& getGlyphDataBuffer() const noexcept { return glyphSSBO; }

			ENGINE_INLINE auto getGlyphIndex(uint32 glyph) { return glyphIndexToLoadedIndex[glyph]; }

			ENGINE_INLINE auto& getManager() const noexcept {
				ENGINE_DEBUG_ASSERT(ftFace->generic.data, "No font manager assigned to glyph set. This is a bug.");
				return *reinterpret_cast<class FontManager*>(ftFace->generic.data);
			}

			void updateDataBuffer();

			/**
			 * Populate a ShapedString with glyph position and bounds info.
			 */
			void shapeString(class ShapedString& str);

			/**
			 * Gets the font specified line height.
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

			ENGINE_INLINE auto getAscent() const noexcept { return ftSize->metrics.ascender * mscale; }

		private:
			void initMaxGlyphSize();
	};
}
