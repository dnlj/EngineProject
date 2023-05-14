#pragma once

// Engine
#include <Engine/ArrayView.hpp>


namespace Engine::UI {
	class ShapeGlyph;

	class Caret {
		public:
			constexpr static uint32 invalid = 0xFFFFFFFF;

			uint32 index;
			float32 pos;

			ENGINE_INLINE Caret(const uint32 index = invalid, const float32 pos = 0) noexcept : index{index}, pos{pos} {}
			ENGINE_INLINE bool valid() const noexcept { return index != invalid; }
			ENGINE_INLINE friend bool operator==(const Caret& a, const Caret& b) noexcept { return a.index == b.index; }
	};

	Caret getCaretInLine(const float32 pos, ArrayView<const ShapeGlyph> glyphs);
}
