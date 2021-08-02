#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Gui/FontGlyphSet.hpp>


namespace Engine::Gui {
	using Font = class FontGlyphSet*;

	class ShapeGlyph {
		public:
			uint32 index;
			glm::vec2 offset;
			glm::vec2 advance;
	};

	class ShapedString {
		private:
			std::string str;
			std::vector<ShapeGlyph> glyphs;
			Font font;

		public:
			ShapedString() = default;
			ShapedString(const ShapedString&) = delete;

			ENGINE_INLINE const auto& getGlyphShapeData() const noexcept { return glyphs; }
			ENGINE_INLINE auto& getGlyphShapeDataMutable() noexcept { return glyphs; }

			ENGINE_INLINE ShapedString& operator=(const char* other) { str = other; return *this; }
			ENGINE_INLINE ShapedString& operator=(std::string_view other) { str = other; return *this; }
			ENGINE_INLINE ShapedString& operator=(const std::string& other) { str = other; return *this; }
			ENGINE_INLINE ShapedString& operator=(std::string&& other) { str = std::move(other); return *this; }
			ENGINE_INLINE const std::string& getString() const noexcept { return str; }

			ENGINE_INLINE void setFont(Font f) noexcept { font = f; }
			ENGINE_INLINE Font getFont() const noexcept { return font; }

			ENGINE_INLINE void shape() { font->shapeString(*this); }
	};
}
