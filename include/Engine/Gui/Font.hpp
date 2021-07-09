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
#include <Engine/Graphics/Texture.hpp>


namespace Engine::Gui {
	class Font {
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

		public:
			// TODO: private
			FT_Face face = nullptr;
			hb_font_t* font = nullptr;
			
			GLuint glyphSSBO = 0;
			GLsizei glyphSSBOSize = 0;
			FlatHashMap<uint32, uint32> glyphIndexToLoadedIndex;
			std::vector<GlyphData> glyphData;
			std::vector<GlyphMetrics> glyphMetrics;
			
			Texture2D glyphTex;
			glm::vec2 maxFace; // TODO: rename - maxGlyphSize
			int nextGlyphIndex = 0; // TODO: glyph index recycling

		public:
			Font();
			Font(FT_Library ftlib);
			Font(const Font&) = delete;
			Font(Font&&) = delete;
			~Font();

			void init(FT_Library ftlib);

		private:
			void initMaxGlyphSize();
	};
}
