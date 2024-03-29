// FreeType
#include <freetype/ftsizes.h>

// Harfbuzz
#include <hb-ft.h>

// GLM
#include <glm/gtx/component_wise.hpp>

// Engine
#include <Engine/UI/FontManager.hpp>
#include <Engine/UI/FontGlyphSet.hpp>
#include <Engine/UI/ShapedString.hpp>
#include <Engine/UI/common.hpp>


namespace Engine::UI {
	FontGlyphSet::FontGlyphSet() {
	}

	FontGlyphSet::~FontGlyphSet() {
		hb_font_destroy(hbFont);
		
		if (const auto err = FT_Done_Size(ftSize)) [[unlikely]] {
			ENGINE_ERROR("FreeType error: ", getFreeTypeErrorString(err));
		}

		if (const auto err = FT_Done_Face(ftFace)) [[unlikely]] {
			ENGINE_ERROR("FreeType error: ", getFreeTypeErrorString(err));
		}
	}

	void FontGlyphSet::init(FT_Face face, int32 size) {
		// Setup FreeType
		FT_Reference_Face(face);
		ftFace = face;

		if (const auto err = FT_New_Size(face, &ftSize)) [[unlikely]] {
			ENGINE_ERROR("FreeType error: ", getFreeTypeErrorString(err));
		}

		if (const auto err = FT_Activate_Size(ftSize)) [[unlikely]] {
			ENGINE_ERROR("FreeType error: ", getFreeTypeErrorString(err));
		}

		if (const auto err = FT_Set_Pixel_Sizes(face, size, size)) [[unlikely]] {
			ENGINE_ERROR("FreeType error: ", getFreeTypeErrorString(err));
		}

		// Setup HarfBuzz
		//
		// TODO (aHGFyF77):
		// Ideally we would share a single hb_face_t* between all sizes of a face (cached in FT_Face::generic)
		// Unfortunately HarfBuzz does not expose `_hb_ft_font_set_funcs` so our only options
		// are to:
		// 
		//   1. Use `hb_ft_font_set_funcs`, which creates a duplicate FT_Face (completely negating our caching;
		//      in fact would be worse than no cache)
		//   2. Manually implement our own `hb_font_funcs_t` struct (this appears to be what chrome/skia do).
		//   3. Use `hb_ft_font_create` which creates a duplicate `hb_face_t` per face/size combo.
		//   4. Use a single `hb_font_t` for all face/size combos and call `hb_font_set_scale`
		//      which negates the metric caching that hb_font_t does internally
		//   5. Fork/PR HarfBuzz and make a public version of `_hb_ft_font_set_funcs`
		// 
		// At the moment this isnt a big enough issue to bother with so we will just have to pay for a
		// duplicate `hb_face_t` per face/size combo.
		//
		// @See https://github.com/harfbuzz/harfbuzz/issues/651
		// @See https://github.com/harfbuzz/harfbuzz/issues/559
		// @See https://github.com/harfbuzz/harfbuzz/blob/main/src/hb-ft.cc
		//
		hbFont = hb_ft_font_create(face, nullptr);

		// Setup OpenGL
		initMaxGlyphSize();
		{
			glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glyphTexSize);

			// Next pow2 that will contain at least `count` glyphs
			constexpr auto count = 10;
			const auto ideal = std::bit_ceil(static_cast<uint32>(glm::compMax(maxGlyphSize) * count));

			glyphTexSize = std::min(glyphTexSize, static_cast<GLint>(ideal));
		};

		indexBounds = glm::floor(glm::vec2{glyphTexSize, glyphTexSize} / maxGlyphSize);

		glyphTex.setStorage(Gfx::TextureFormat::R8, {glyphTexSize, glyphTexSize});
		glyphTex.setSwizzle(Gfx::TextureChannel::One, Gfx::TextureChannel::One, Gfx::TextureChannel::One, Gfx::TextureChannel::Red);
	}

	void FontGlyphSet::loadGlyph(const uint32 index) {
		ftFace->size = ftSize;
		
		if (const auto err = FT_Load_Glyph(ftFace, index, FT_LOAD_RENDER)) [[unlikely]] {
			ENGINE_ERROR("FreeType error: ", getFreeTypeErrorString(err));
		}
		
		const auto& glyph = *ftFace->glyph;
		const auto& metrics = glyph.metrics;
		auto& met = glyphMetrics.emplace_back();
		met.bearing = {metrics.horiBearingX * mscale, metrics.horiBearingY * -mscale};
		met.size = {metrics.width * mscale, metrics.height * mscale};
		glyphIndexToLoadedIndex[index] = nextGlyphIndex;

		if (glyph.bitmap.width) {
			// TODO: really need to handle glyph FT_Pixel_Mode and alignment(glyph.pitch) here
			const glm::vec2 i = {
				nextGlyphIndex % indexBounds.x,
				nextGlyphIndex / indexBounds.x,
			};
			met.offset = i * maxGlyphSize;

			ENGINE_DEBUG_ASSERT(i.y < indexBounds.y, "Glyph texture index is out of bounds. Should rollover to next texture layer in array.");

			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glyphTex.setSubImage(0, met.offset, met.size, Gfx::PixelFormat::R8, glyph.bitmap.buffer);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		}
		
		//ENGINE_INFO("Loaded glyph ", index, " = ", nextGlyphIndex, " @ ", dat.offset.x, ", ", dat.offset.y);
		++nextGlyphIndex;
	}

	void FontGlyphSet::initMaxGlyphSize() {
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
			FT_MulFix(ftFace->bbox.xMax - ftFace->bbox.xMin, ftFace->size->metrics.x_scale) * mscale,
			FT_MulFix(ftFace->bbox.yMax - ftFace->bbox.yMin, ftFace->size->metrics.y_scale) * mscale
		});

		// See above comment
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

	ENGINE_INLINE void FontGlyphSet::fallback(std::string_view str, hb_glyph_info_t& info, hb_glyph_position_t& pos) {
		// At the momement we only have single char fallbacks, but we could have
		// a cluster of multiple chars map to a single glyph.
		//const char c = str[info.cluster];

		//
		// TODO: this isn't really a good way to handle tabs. They need to be
		// handled at render time because we dont know WHERE the tab stops
		// actually are. Consider: "\thello" and "h\tello".
		//
		// You would expect:
		// `    hello`
		// `h   ello`
		// 
		// But with this you will get:
		// `    hello`
		// `h    ello`
		//
		// In the second case the \t should only be three wide. With this impl it is always four.
		//
		//if (c == '\t') {
		//	if (hb_font_get_glyph(hbFont, 0x20, 0, &info.codepoint)) {
		//		pos.x_advance = 4*hb_font_get_glyph_h_advance(hbFont, info.codepoint);
		//	}
		//}
	}
	
	template<class ShapeGlyphCont>
	void FontGlyphSet::shapeStringImpl(std::string_view str, ShapeGlyphCont& glyphs, Bounds& bounds) {
		auto buffer = getManager().getWorkingBuffer();
		ftFace->size = ftSize;

		hb_buffer_clear_contents(buffer);
		hb_buffer_add_utf8(buffer, str.data(), static_cast<int>(str.size()), 0, -1);
		hb_buffer_guess_segment_properties(buffer); // TODO: Should we handle this ourself? Should this be set in fontmanager?
		hb_shape(hbFont, buffer, nullptr, 0);

		const auto sz = hb_buffer_get_length(buffer);
		const auto infoArr = hb_buffer_get_glyph_infos(buffer, nullptr);
		const auto posArr = hb_buffer_get_glyph_positions(buffer, nullptr);

		const auto basei = glyphs.size();
		glyphs.resize(basei + sz);

		bounds = {{0,0}, {0,0}};
		glm::vec2 cursor = {}; // Used for calc bounds

		for (uint32 i = 0; i < sz; ++i) {
			auto& info = infoArr[i];
			auto& pos = posArr[i];

			if (!info.codepoint) {
				fallback(str, info, pos);
			}

			if (!info.codepoint) {
				const auto c = str[info.cluster];
				ENGINE_WARN("Missing one or more glyphs for character at index ", info.cluster, " = ",  static_cast<int>(c)," (", c, ")");
			}

			if (!isGlyphLoaded(info.codepoint)) {
				loadGlyph(info.codepoint);
			}

			const auto gi = glyphIndexToLoadedIndex[info.codepoint];
			const auto& met = glyphMetrics[gi];

			auto& glyph = glyphs[basei + i];
			glyph = {
				//.index = info.codepoint, // info.codepoint is a glyph index not a actual code point
				.glyphId = gi,
				.cluster = info.cluster,
				.offset = glm::vec2{pos.x_offset, pos.y_offset} * (1.0f/64) + met.bearing,
				.advance = glm::vec2{pos.x_advance, pos.y_advance} * (1.0f/64),
			};

			{ // Update bounds
				const auto min = cursor + glyph.offset;
				const auto max = min + met.size;

				bounds.min = glm::min(bounds.min, min);
				bounds.max = glm::max(bounds.max, max);

				cursor += glyph.advance;
			}
		}
	}

	template void FontGlyphSet::shapeStringImpl(std::string_view str, std::vector<ShapeGlyph>& glyphs, Bounds& bounds);
	template void FontGlyphSet::shapeStringImpl(std::string_view str, RingBuffer<ShapeGlyph>& glyphs, Bounds& bounds);
}

