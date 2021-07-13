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
#include <Engine/Gui/ShapedString.hpp>


namespace Engine::Gui {
	// TODO: name?
	/**
	 * TODO: doc.
	 *
	 * It looks like we dont need activate size unless we are loading a glyph?
	 * HarfBuzz looks like it stores its own size metrics at the time you create the font. See hb_ft_font_changed. 
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

			/* TODO: it looks like we should be caching a hb_face_t object and share that between multiple hb_font_t objects.
			* since hb_face_t objects can be large
			* 
			* See
			* https://harfbuzz.github.io/fonts-and-faces.html
			* def hb_ft_font_create
			* This seems to be what hb_ft_face_create_cached is for? we can just do it ourselfs though.
			*/
			hb_font_t* hbFont;

			hb_buffer_t* workingBuffer; // TODO: own by font manager, shared by fonts

			GLuint glyphSSBO = 0;
			GLsizei glyphSSBOSize = 0;
		public: FlatHashMap<uint32, uint32> glyphIndexToLoadedIndex; private: // TODO: private
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

			void init(FT_Face face, int32 size, hb_buffer_t* buff);

			void loadGlyph(const uint32 index);

			ENGINE_INLINE bool isGlyphLoaded(const uint32 index) const noexcept { return glyphIndexToLoadedIndex.contains(index); };

			ENGINE_INLINE void ensureGlyphLoaded(const uint32 index) { if (!isGlyphLoaded(index)) { loadGlyph(index); } }

			ENGINE_INLINE const auto& getGlyphTexture() const noexcept { return glyphTex; }

			ENGINE_INLINE const auto& getGlyphDataBuffer() const noexcept { return glyphSSBO; }

			void updateDataBuffer();

			void shapeString(ShapedString& str);

		private:
			void initMaxGlyphSize();
	};
}
