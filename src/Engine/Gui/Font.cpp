// Harfbuzz
#include <hb-ft.h>

// Engine
#include <Engine/Gui/Font.hpp>


namespace Engine::Gui {
	Font::Font() {
	}

	Font::Font(FT_Library ftlib) {
		init(ftlib);
	}

	Font::~Font() {
		glDeleteBuffers(1, &glyphSSBO);

		hb_font_destroy(font);

		if (const auto err = FT_Done_Face(face)) {
			ENGINE_ERROR("FreeType error: ", err); // TODO: actual error
		}
	}

	
	void Font::init(FT_Library ftlib) {
		if (const auto err = FT_New_Face(ftlib, "assets/arial.ttf", 0, &face)) {
			ENGINE_ERROR("FreeType error: ", err); // TODO: actual error
		}

		if (const auto err = FT_Set_Pixel_Sizes(face, 0, 32)) {
			ENGINE_ERROR("FreeType error: ", err); // TODO: actual error
		}

		font = hb_ft_font_create_referenced(face);

		GLint texSize;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);
		ENGINE_INFO("Maximum texture size: ", texSize);

		initMaxGlyphSize();

		// TODO: may want to limit base on face size? for small font sizes 4096 might be excessive.
		texSize = std::min(texSize, 4096);
		
		indexBounds = glm::floor(glm::vec2{texSize, texSize} / maxGlyphSize);
		
		glyphTex.setStorage(TextureFormat::R8, {texSize, texSize});

		glCreateBuffers(1, &glyphSSBO);
	}

	void Font::loadGlyph(const uint32 index) {
		if (const auto err = FT_Load_Glyph(face, index, FT_LOAD_RENDER)) [[unlikely]] {
			ENGINE_ERROR("FreeType error: ", err); // TODO: actual error
		}
		
		const auto& glyph = *face->glyph;
		const auto& metrics = glyph.metrics;

		auto& met = glyphMetrics.emplace_back();
		met.index = nextGlyphIndex;
		met.bearing = {metrics.horiBearingX * mscale, metrics.horiBearingY * -mscale};

		auto& dat = glyphData.emplace_back();
		dat.size = {metrics.width * mscale, metrics.height * mscale};
		glyphIndexToLoadedIndex[index] = nextGlyphIndex;
		++nextGlyphIndex;

		if (glyph.bitmap.width) {
			const glm::vec2 i = {
				met.index % indexBounds.x,
				met.index / indexBounds.x,
			};
			dat.offset = glm::vec3{i * maxGlyphSize, 0};

			ENGINE_DEBUG_ASSERT(i.y < indexBounds.y, "Glyph texture index is out of bounds. Should rollover to next texture layer in array.");

			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glyphTex.setSubImage(0, dat.offset, dat.size, PixelFormat::R8, glyph.bitmap.buffer);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		}
	}

	void Font::initMaxGlyphSize() {
		// In practice most glyphs are much smaller than `face->bbox` (every ascii char is < 1/2 the size depending on font)
		// It would be better to use a packing algorithm, but then we would want
		// to periodically repack (glCopy*, not reload) as glyphs are unloaded which complicates things.
		// Still very doable. Just requires more work.
		// 
		// I think this may be subtly incorrect.
		// 
		// I think this is the bounding box that could fit ALL glyphs at the same
		// time. Not a box that is large enough to contain any given glyph.
		// 
		// For example:
		// We have one glyph whos outline is bound by [0, 10] (10u wide).
		// We have another glyph whos outline is not positioned at the origin
		// so it is bound by [80, 100] (20u wide).
		// The bbox needed to contain these is only 20u wide, but the bbox
		// needed to contain BOTH is [0, 100] (100u wide).
		// I think this is what face->bbox is?
		// 
		// After some testing the above does appear to be correct.
		// To calc the bbox that we need (maximum size of any single glyph) takes
		// 250ms/150ms (debug/release) per 4000 glyphs with a 5820k @ 4.1GHz.
		// The max number of glyphs a font can have is 65,535 which would take ~4.1s
		//
		// If we calc the bbox in font units we should be able to only do
		// it once per font and then scale by pixel size (face.size.metrics.x/y_scale?)
		//
		// Although the size difference doesnt seem to be that large. Maybe its not worth doing?
		//
		maxGlyphSize = glm::ceil(glm::vec2{
			FT_MulFix(face->bbox.xMax - face->bbox.xMin, face->size->metrics.x_scale) * mscale,
			FT_MulFix(face->bbox.yMax - face->bbox.yMin, face->size->metrics.y_scale) * mscale
		});

		// Find minimum bounding box that can contain any glyph
		//glm::vec2 maxGlyph = {};
		//const auto startT = Clock::now();
		//for (int i = 0; FT_Load_Glyph(face, i, FT_LOAD_DEFAULT) == 0; ++i) {
		//	FT_Glyph glyph;
		//	FT_BBox bbox;
		//	if (FT_Get_Glyph(face->glyph, &glyph)) { ENGINE_WARN("Oh no!"); break; }
		//	FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_GRIDFIT, &bbox);
		//	FT_Done_Glyph(glyph);
		//	maxGlyph.x = std::max(maxGlyph.x, (bbox.xMax - bbox.xMin) * mscale);
		//	maxGlyph.y = std::max(maxGlyph.y, (bbox.yMax - bbox.yMin) * mscale);
		//}
		//const auto endT = Clock::now();
		//ENGINE_LOG("Time: ", Clock::Milliseconds{endT-startT}.count());
		//ENGINE_LOG("Max Glyph: ", maxGlyph.x, ", ", maxGlyph.y);
		//ENGINE_LOG("Max Face: ", maxFace.x, ", ", maxFace.y);
	}
}
