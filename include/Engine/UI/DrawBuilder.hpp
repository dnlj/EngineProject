#pragma once

// Engine
#include <Engine/ArrayView.hpp>
#include <Engine/UI/Bounds.hpp>
#include <Engine/UI/FontManager.hpp>
#include <Engine/UI/ShapedString.hpp>
#include <Engine/Gfx/resources.hpp>


namespace Engine::UI {
	class DrawGroupManager {
		protected:
			struct DrawGroup {
				int32 offset = {}; // VBO offset
				int32 count = {}; // VBO elements
				Bounds clip = {}; // Empty if clipping is done 100% in software for this group.
				Gfx::TextureHandleGeneric tex = {};
			};

			struct Vertex {
				glm::vec2 pos;
				glm::u16vec2 texCoord;
				glm::u8vec4 color; // TODO: could be moved to a per-tri attribute buffer to reduce upload cost. bench and test.
				glm::u8 layer; // TODO: could be moved to a per-tri attribute buffer to reduce upload cost. bench and test.
				char _unused[3];
			}; static_assert(sizeof(Vertex) == 8+4+4 +4);

			using Element = uint16;

		private:
			/* Render state */
			Gfx::TextureHandleGeneric activeTexture; // TODO: rename just texture
			glm::vec4 color = {1,1,1,1};
			int32 lastClipOffset = 0; // VBO offset of current clipping group
			std::vector<Bounds> clipStack;
			std::vector<DrawGroup> drawGroups;
			std::vector<Vertex> vertData;
			std::vector<Element> elemData;

		public:
			ENGINE_INLINE void setColor(glm::vec4 color) noexcept { this->color = color * 255.0f; nextDrawGroup(); }
			ENGINE_INLINE auto getColor() const noexcept { return color; }

			ENGINE_INLINE void setTexture(Gfx::TextureHandleGeneric tex) noexcept { activeTexture = tex; nextDrawGroup(); }
			ENGINE_INLINE auto getTexture() const noexcept { return activeTexture; }

			void pushClip();
			void popClip();
			void setClip(Bounds bounds); // TODO: remove or change to use pop,push pattern
			ENGINE_INLINE const auto& getClip() const noexcept { ENGINE_DEBUG_ASSERT(clipStack.size() > 0); return clipStack.back(); }
			ENGINE_INLINE const auto& getRootClip() const noexcept { ENGINE_DEBUG_ASSERT(clipStack.size() > 0); return clipStack.front(); }

			ENGINE_INLINE const auto& getVertexData() const noexcept { return vertData; }
			ENGINE_INLINE const auto& getElementData() const noexcept { return elemData; }
			ENGINE_INLINE const auto& getDrawGroups() const noexcept { return drawGroups; }
			
			void finish();
			void reset(Bounds clip);

			void makeHardwareClip();

			void addVertex(glm::vec2 pos, glm::vec2 texCoord, uint8 layer = 0) {
				vertData.push_back({
					.pos = pos,
					.texCoord = texCoord * 65535.0f,
					.color = color,
					.layer = layer, // TOOD: what is this? why do we need it?
				});
			}

			ENGINE_INLINE void addElements(uint32 i1, uint32 i2, uint32 i3) {
				elemData.push_back(i1); elemData.push_back(i2); elemData.push_back(i3);
			}

		private:
			void nextDrawGroup();
	};
	// TODO: we really should separate the buffer building and draw/opengl objects. But since we only support opengl atm not a lot to gain.
	class DrawBuilder : protected DrawGroupManager {
		private:
			/* Render state */
			Gfx::Texture2D defaultTexture; /** Default blank (white) texture */
			Font font = nullptr;
			glm::vec2 view = {};
			Gfx::ShaderRef polyShader;

			// TODO: use Gfx:: types
			glm::vec2 drawOffset; /* The position offset to use for rendering */
			GLuint polyVAO = 0;
			GLuint polyVBO = 0;
			GLsizei polyVBOCapacity = 0;
			GLuint polyEBO = 0;
			GLsizei polyEBOCapacity = 0;
			
		public: // TODO: private
			FontManager fontManager2;

		public:
			DrawBuilder(Gfx::ShaderLoader& shaderLoader, Gfx::TextureLoader& textureLoader);
			~DrawBuilder();

			using DrawGroupManager::setColor;
			using DrawGroupManager::setClip;

			void setOffset(glm::vec2 offset) noexcept { drawOffset = offset; }
			void resize(glm::vec2 view);
			void finish();
			void reset();
			void draw();

			// TODO: probably move this state stuff into base class to avoid changing in draw* functions
			ENGINE_INLINE void setFont(Font f) noexcept { font = f; }

			void drawTexture(Gfx::TextureHandle2D tex, glm::vec2 pos, glm::vec2 size);
			 
			/**
			 * Draws a convex polygon from a ordered set of perimeter points.
			 * If the points are not in order the results are undefined.
			 *
			 * May create a new hardware clipping group.
			 * 
			 * @param points Three or more ordered perimeter points.
			 */
			void drawPoly(ArrayView<const glm::vec2> points);

			/**
			 * Draws a rectangle from a position and size.
			 */
			void drawRect(glm::vec2 pos, glm::vec2 size);

			/**
			 * Draws a line between two points.
			 */
			void drawLine(glm::vec2 a, glm::vec2 b, float32 width);

			/**
			 * Draws a string of glyphs.
			 * @return The accumulated advance of the string drawn.
			 */
			glm::vec2 drawString(glm::vec2 pos, Font font, ArrayView<const ShapeGlyph> glyphs);

			ENGINE_INLINE glm::vec2 drawString(glm::vec2 pos, const ShapedString* fstr) {
				return drawString(pos, fstr->getFont(), fstr->getGlyphShapeData());
			}


		private:
			// TODO: rm - dont we need an addElement here also?
			//ENGINE_INLINE void drawTri(glm::vec2 a, glm::vec2 b, glm::vec2 c) {
			//	addVertex(a); addVertex(b); addVertex(c);
			//}

	};
}
