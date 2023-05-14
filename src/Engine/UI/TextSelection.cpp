// Engine
#include <Engine/UI/TextSelection.hpp>
#include <Engine/UI/FontGlyphSet.hpp>


namespace Engine::UI {
	Caret getCaretInLine(const float32 x, ArrayView<const ShapeGlyph> glyphs) {
		Caret result = {0,0};
		if (glyphs.empty()) { return result; }

		// Use glyph advances to approximate glyph bbox.
		// To do this "correctly" we would have to fully calculate the glyph
		// bbox(adv+off+width), which could overlap glyphs leading to strange selections
		// where you end up selecting a code point that occurs visually after but byte
		// order before. Unless a problem arises, using advances is better in my opinion
		// because of more obvious selection.
		// 
		// The multipler for advance used here is just a guess based on observation and
		// feel. A value around 0.6 feels about right. 0.5 is to small. Im not sure how other text
		// engines handle selection. Probably worth looking into to get 100% native feel.
		constexpr float32 threshold = 0.6f;
		const auto end = glyphs.cend();
		auto cur = glyphs.cbegin();

		for (;cur != end; ++cur) {
			if (result.pos + cur->advance.x * threshold > x) {
				result.index = cur->cluster;
				break;
			}

			result.pos += cur->advance.x;
		}

		if (cur == end) {
			result.index = glyphs.back().cluster + 1;
		}

		return result;
	}
}
