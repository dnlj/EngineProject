#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/UI/FontGlyphSet.hpp>


namespace Engine::UI {
	class ShapedString {
		private:
			std::string str;
			std::vector<ShapeGlyph> glyphs;
			Font font = nullptr;
			Bounds bounds;

		public:
			ShapedString() = default;
			ShapedString(const ShapedString&) = delete;
			ShapedString(ShapedString&& other) = default;

			void clear() {
				str.clear();
				glyphs.clear();
				font = nullptr;
				bounds = {};
			}

			ENGINE_INLINE const auto& getGlyphShapeData() const noexcept { return glyphs; }

			ENGINE_INLINE ShapedString& operator=(ShapedString&& other) = default;
			ENGINE_INLINE ShapedString& operator=(const char* other) { str = other; return *this; }
			ENGINE_INLINE ShapedString& operator=(std::string_view other) { str = other; return *this; }
			ENGINE_INLINE ShapedString& operator=(const std::string& other) { str = other; return *this; }
			ENGINE_INLINE ShapedString& operator=(std::string&& other) { str = std::move(other); return *this; }
			ENGINE_INLINE const std::string& getString() const noexcept { return str; }
			ENGINE_INLINE std::string& getStringMutable() noexcept { return str; }

			ENGINE_INLINE void setFont(Font f) noexcept { font = f; }
			ENGINE_INLINE Font getFont() const noexcept { return font; }

			ENGINE_INLINE void setBounds(Bounds b) { bounds = b; }
			ENGINE_INLINE const auto& getBounds() const noexcept { return bounds; }

			ENGINE_INLINE void shape() {
				glyphs.clear();
				bounds = {};
				font->shapeString(str, glyphs, bounds);
			}
	};
}
