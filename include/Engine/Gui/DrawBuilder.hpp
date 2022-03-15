#pragma once

// Engine
#include <Engine/ArrayView.hpp>
#include <Engine/Gui/Bounds.hpp>
#include <Engine/Gui/FontManager.hpp>
#include <Engine/Gui/ShapedString.hpp>


namespace Engine::Gui {
	// TODO: we really should separate the buffer building and draw/opengl objects. But since we only support opengl atm not a lot to gain.
	class DrawBuilder {
		private:
			struct PolyDrawGroup {
				int32 zindex = {};
				int32 offset = {};
				int32 count = {};
				Bounds clip = {};
				TextureHandle2D tex = {};
			};

			struct PolyVertex {
				glm::vec2 pos;
				glm::u16vec2 texCoord;
				glm::u8vec4 color;
			}; static_assert(sizeof(PolyVertex) == 8+4+4);

			struct GlyphDrawGroup {
				int32 zindex = {};
				int32 offset = {};
				int32 count = {};
				Bounds clip = {};
				Font font = {};
			};

			struct GlyphVertex {
				glm::vec2 pos;
				glm::u8vec4 color;
				uint32 index;
			}; static_assert(sizeof(GlyphVertex) == 8+4+4);

		private:
			/* Polygon members */
			GLuint polyVAO = 0;
			GLuint polyVBO = 0;
			GLsizei polyVBOCapacity = 0;
			std::vector<PolyDrawGroup> polyDrawGroups;
			std::vector<PolyVertex> polyVertexData;
			ShaderRef polyShader;
			
			GLuint polyEBO = 0;
			GLsizei polyEBOCapacity = 0;
			std::vector<uint16> polyElementData;

			/* Glyph members */
			GLuint glyphVAO = 0;
			GLuint glyphVBO = 0;
			GLsizei glyphVBOCapacity = 0;
			std::vector<GlyphDrawGroup> glyphDrawGroups;
			std::vector<GlyphVertex> glyphVertexData;
			ShaderRef glyphShader;

			/* Render state */
			std::vector<Bounds> clipStack;
			TextureHandle2D activeTexture;
			Texture2D defaultTexture; /** Default blank (white) texture */
			Font font = nullptr;
			int32 zindex = -1;
			glm::vec2 view = {};
			
		public: // TODO: private
			glm::vec2 drawOffset; /* The offset to use for rendering */
			FontManager fontManager2;

		public:
			DrawBuilder(ShaderManager& shaderManager, TextureManager& textureManager);
			~DrawBuilder();

			void resize(glm::vec2 view);
			void draw();
			void reset();
			void finish();
			void nextDrawGroupPoly();
			void nextDrawGroupGlyph();

			void pushClip();
			void popClip();
			void setClip(Bounds bounds);

			void setFont(Font f) { font = f; }

			void drawTexture(TextureHandle2D tex, glm::vec2 pos, glm::vec2 size);

			/**
			 * Draws a convex polygon from a ordered set of perimeter points.
			 * If the points are not in order the results are undefined.
			 * 
			 * @param points Three or more ordered perimeter points.
			 * @param color The color of the polygon.
			 */
			void drawPoly(ArrayView<const glm::vec2> points, glm::vec4 color);

			/**
			 * Draws a rectangle from a position and size.
			 */
			void drawRect(glm::vec2 pos, glm::vec2 size, glm::vec4 color);

			/**
			 * Draws a line between two points.
			 */
			void drawLine(glm::vec2 a, glm::vec2 b, float32 width, glm::vec4 color);

			ENGINE_INLINE void drawString(glm::vec2 pos, const ShapedString* fstr, glm::vec4 color) {
				drawString(pos, color, fstr->getFont(), fstr->getGlyphShapeData());
			}
			void drawString(glm::vec2 pos, glm::vec4 color, Font font, ArrayView<const ShapeGlyph> glyphs);

		private:
			ENGINE_INLINE void drawVertex(glm::vec2 pos, glm::vec2 texCoord, glm::vec4 color = {1,1,1,1}) {
				polyVertexData.push_back({
					.pos = pos + drawOffset,
					.texCoord = texCoord * 65535.0f,
					.color = color * 255.0f,
				});
			}

			ENGINE_INLINE void drawVertex(glm::vec2 pos, glm::vec4 color) {
				drawVertex(pos, {}, color);
			}

			ENGINE_INLINE void drawTri(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec4 color) {
				drawVertex(a, color); drawVertex(b, color); drawVertex(c, color);
			}

			ENGINE_INLINE void addPolyElements(uint32 i1, uint32 i2, uint32 i3) {
				polyElementData.push_back(i1); polyElementData.push_back(i2); polyElementData.push_back(i3);
			}

	};
}
