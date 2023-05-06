// Engine
#include <Engine/UI/DrawBuilder.hpp>
#include <Engine/Gfx/ShaderManager.hpp>


namespace Engine::UI {
	void DrawGroupManager::pushClip() {
		clipStack.push_back(clipStack.back());
	}
			
	void DrawGroupManager::popClip() {
		ENGINE_DEBUG_ASSERT(!clipStack.empty(), "Attempting to pop empty clipping stack");
		clipStack.pop_back();
		lastClipOffset = static_cast<int32>(elemData.size());
		nextDrawGroup();
	}

	void DrawGroupManager::setClip(Bounds bounds) {
		auto& curr = clipStack.back();

		// Avoid splitting groups with the same clipping
		if (curr != bounds) {
			lastClipOffset = static_cast<int32>(elemData.size());
		}

		// Set clip
		curr = (clipStack.end() - 2)->intersect(bounds);
		curr.max = glm::max(curr.min, curr.max);

		nextDrawGroup();
	}
	void DrawGroupManager::nextDrawGroup() {
		ENGINE_DEBUG_ASSERT(!drawGroups.empty());
		const auto sz = static_cast<int32>(elemData.size());
		auto& prev = drawGroups.back();
		prev.count = sz - prev.offset;
		ENGINE_DEBUG_ASSERT(prev.count >= 0);

		const auto setup = [&](DrawGroup& group) ENGINE_INLINE {
			group.offset = sz;
			group.tex = activeTexture;
			lastClipOffset = group.offset;
		};
		
		// Figure out if we need a new group, can reuse an empty group, or extend the previous group
		if (prev.count == 0) {
			setup(prev = {});
		} else if (false
			|| (prev.tex != activeTexture)
			// ...
			) {
			setup(drawGroups.emplace_back());
		} else {
			// No group change needed, extend the previous group
		}
	}
	
	void DrawGroupManager::reset(Bounds clip) {
		clipStack.clear();
		clipStack.push_back(clip);
		lastClipOffset = 0;

		elemData.clear();
		vertData.clear();
		drawGroups.clear();
		drawGroups.emplace_back();
	}

	void DrawGroupManager::finish() {
		// Populate count of last draw groups
		if (auto last = drawGroups.rbegin(); last != drawGroups.rend()) {
			last->count = static_cast<int32>(elemData.size()) - last->offset;

			// This will be true 100% of the time based on how we currently push/pop clips.
			// Thats technically not part of the DrawGroupManager though so we still need to calc.
			if (last->count == 0) {
				drawGroups.pop_back();
			}
		}
		ENGINE_DEBUG_ASSERT(clipStack.size() == 1, "Mismatched push/pop clip");
	}

	void DrawGroupManager::makeHardwareClip() {
		auto* last = &drawGroups.back();
		if (!last->clip.empty()) { return; }

		// We need to split the previous group
		if (last->offset != lastClipOffset) {
			ENGINE_DEBUG_ASSERT(lastClipOffset > last->offset);
			auto* next = &drawGroups.emplace_back(*last);
			next->offset = lastClipOffset;
			last->count = next->offset - last->offset;
			last = next;
		}

		last->clip = clipStack.back();
	}
}


namespace Engine::UI {
	DrawBuilder::DrawBuilder(Gfx::ShaderLoader& shaderLoader, Gfx::TextureLoader& textureLoader) {
		polyShader = shaderLoader.get("shaders/gui_poly");
		glProgramUniform1i(polyShader->get(), 1, 0);

		{
			constexpr static GLuint bindingIndex = 0;
			GLuint attribLocation = -1;

			glCreateVertexArrays(1, &polyVAO);

			glCreateBuffers(1, &polyEBO);
			glVertexArrayElementBuffer(polyVAO, polyEBO);

			glCreateBuffers(1, &polyVBO);
			glVertexArrayVertexBuffer(polyVAO, bindingIndex, polyVBO, 0, sizeof(Vertex));

			glEnableVertexArrayAttrib(polyVAO, ++attribLocation);
			glVertexArrayAttribBinding(polyVAO, attribLocation, bindingIndex);
			glVertexArrayAttribFormat(polyVAO, attribLocation, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));

			glEnableVertexArrayAttrib(polyVAO, ++attribLocation);
			glVertexArrayAttribBinding(polyVAO, attribLocation, bindingIndex);
			glVertexArrayAttribFormat(polyVAO, attribLocation, 2, GL_UNSIGNED_SHORT, GL_TRUE, offsetof(Vertex, texCoord));

			glEnableVertexArrayAttrib(polyVAO, ++attribLocation);
			glVertexArrayAttribBinding(polyVAO, attribLocation, bindingIndex);
			glVertexArrayAttribFormat(polyVAO, attribLocation, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(Vertex, color));
		}

		{
			using namespace Gfx;

			// TODO: rm - doesn't the resource manager already provide a default texture?
			Image img{PixelFormat::RGB8, {1,1}};
			memset(img.data(), 0xFF, img.sizeBytes());
			defaultTexture.setStorage(TextureFormat::RGB8, img.size());
			defaultTexture.setImage(img);
		}

		reset();
		setTexture(defaultTexture);
	}

	DrawBuilder::~DrawBuilder() {
		glDeleteVertexArrays(1, &polyVAO);
		glDeleteBuffers(1, &polyEBO);
		glDeleteBuffers(1, &polyVBO);
	}

	void DrawBuilder::resize(glm::vec2 viewport) {
		view = viewport;
		const auto view2 = 2.0f / view;
		glProgramUniform2fv(polyShader->get(), 0, 1, &view2.x);
	}

	void DrawBuilder::finish() {
		DrawGroupManager::finish();
	}

	void DrawBuilder::reset() {
		DrawGroupManager::reset({{0,0}, view});
	}

	void DrawBuilder::draw() {
		// Update font buffers
		fontManager2.updateAllFontDataBuffers();

		// Update poly vertex/element buffer
		{ // TODO: pull into function
			const auto& vertData = getVertexData();
			const auto& elemData = getElementData();
			if (const auto count = vertData.size()) {
				{
					const auto size = count * sizeof(Vertex);
					if (size > polyVBOCapacity) {
						polyVBOCapacity = static_cast<GLsizei>(vertData.capacity() * sizeof(Vertex));
						glNamedBufferData(polyVBO, polyVBOCapacity, nullptr, GL_DYNAMIC_DRAW);
					}
					glNamedBufferSubData(polyVBO, 0, size, vertData.data());
				}
				{
					const auto size = elemData.size() * sizeof(Element);
					if (size > polyEBOCapacity) {
						polyEBOCapacity = static_cast<GLsizei>(elemData.capacity() * sizeof(Element));
						glNamedBufferData(polyEBO, polyEBOCapacity, nullptr, GL_DYNAMIC_DRAW);
					}
					glNamedBufferSubData(polyEBO, 0, size, elemData.data());
				}
			}
		}

		const auto scissor = [y=view.y](const Bounds& bounds) ENGINE_INLINE {
			glScissor(
			      static_cast<int32>(bounds.min.x),
			      static_cast<int32>(y - bounds.max.y),
			      static_cast<int32>(bounds.getWidth()),
			      static_cast<int32>(bounds.getHeight())
			);
		};

		// Draw polys
		for (auto const& group : getDrawGroups()) {
			ENGINE_DEBUG_ASSERT(group.count > 0);

			glBindVertexArray(polyVAO);
			glUseProgram(polyShader->get());
			Gfx::TextureHandleGeneric activeTex = {};

			// TODO: We could save on a few scissor calls here if we track all clips and
			//       have a separate isHardware flag. This would allow us to only revert to
			//       the root clip if the next clip is outside the current scissor. For
			//       example, we could draw all sub panel and the only reset to root clip
			//       once all sub panel are done. With the current implementation we will
			//       have an unneeded scissor(root) then a scissor(sub) where we should
			//       really only need the scissor(sub), the scissor (root) is not needed
			//       because we are in a sub panel.
			if (!group.clip.empty()) {
				scissor(group.clip);
			} else {
				scissor(getRootClip());
			}

			if (group.tex != activeTex) {
				activeTex = group.tex;
				glBindTextureUnit(0, activeTex.get());
			}

			glDrawElements(GL_TRIANGLES,
				group.count,
				sizeof(Element) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
				(void*)(group.offset * sizeof(Element))
			);
		}
	}

	void DrawBuilder::drawTexture(Gfx::TextureHandle2D tex, glm::vec2 pos, glm::vec2 size) {
		const auto old = getTexture();
		setTexture(tex);
		drawRect(pos, size);
		setTexture(old);
	}

	void DrawBuilder::drawPoly(ArrayView<const glm::vec2> points) {
		ENGINE_DEBUG_ASSERT(points.size() >= 3, "Must have at least three points");
		makeHardwareClip();

		const auto base = static_cast<int32>(getVertexData().size());
		const auto psz = points.size();
		addVertex(points[0] + drawOffset, {});
		addVertex(points[1] + drawOffset, {});

		for (int i = 2; i < psz; ++i) {
			addVertex(points[i] + drawOffset, {});
			addElements(base, base + i - 1, base + i);
		}
	}

	void DrawBuilder::drawRect(glm::vec2 pos, glm::vec2 size) {
		const auto pvdSz = getVertexData().size();
		auto base = static_cast<uint32>(pvdSz);

		pos += drawOffset;

		const auto rect = getClip().intersect({pos, pos+size});
		if (rect.max.x <= rect.min.x || rect.max.y <= rect.min.y) { return; }

		addVertex(rect.min, {0,1});
		addVertex({rect.min.x, rect.max.y}, {0,0});
		addVertex(rect.max, {1,0});
		addVertex({rect.max.x, rect.min.y}, {1,1});

		addElements(base, base+1, base+2);
		addElements(base+2, base+3, base);
	}
	
	void DrawBuilder::drawLine(glm::vec2 a, glm::vec2 b, float32 width) {
		const auto t = glm::normalize(b - a);
		const auto n = width * 0.5f * glm::vec2{-t.y, t.x};
		drawPoly({a - n, a + n, b + n, b - n});
	}

	glm::vec2 DrawBuilder::drawString(glm::vec2 pos, Font font, ArrayView<const ShapeGlyph> glyphs) {
		ENGINE_DEBUG_ASSERT(font != nullptr, "Attempting to draw string with null font.");
		if (glyphs.empty()) { return pos; }

		// TODO (N2s3MidY): this is a weird way to handle fonts. Either have a separate DrawBuilder::setFont function with this being a shortcut for that.
		// or have nextDrawGroupGlyph take a font as param. Passing through member is strange with the current configuration.
		// Maybe that would be best to have properties like color and font set on the object with setColor/setFont.
		// then we have drawString(pos,glyphs). with the current version just being shorthand for that.
		//this->font = font; 
		//pos += drawOffset;
		//nextDrawGroupGlyph();

		const auto& glyphData = font->_debug_getGlyphData(); // TODO: remove this function, see definition for details
		const auto old = getTexture();
		auto base = static_cast<uint32>(getVertexData().size());
		setTexture(font->getGlyphTexture());

		for (const auto& data : glyphs) ENGINE_INLINE_CALLS {
			const uint32 index = font->getGlyphIndex(data.index);
			const glm::vec2 offset = glyphData[index].offset / 4096.0f;
			auto size = glyphData[index].size;
			const auto uvsize = size / 4096.0f;

			const auto p = glm::round(pos + data.offset + drawOffset); // I think this should technically also include the size offset per vert. In practice it does not make a difference (in any tests i have done) and this is faster.
			pos += data.advance;
			const auto& clip = getClip();
			auto orig = Bounds{p, p+size};
			auto uv = Bounds{offset, uvsize};

			if (orig.min.x < clip.min.x) {
				uv.min.x += (clip.min.x - orig.min.x) * (uvsize.x / (orig.max.x - orig.min.x));
				orig.min.x = clip.min.x;
			}
			if (orig.max.x > clip.max.x) {
				uv.max.x -= (orig.max.x - clip.max.x) * (uvsize.x / (orig.max.x - orig.min.x));
				orig.max.x = clip.max.x;
			}
			if (orig.max.x <= orig.min.x) { continue; }

			if (orig.min.y < clip.min.y) {
				uv.min.y += (clip.min.y - orig.min.y) * (uvsize.y / (orig.max.y - orig.min.y));
				orig.min.y = clip.min.y;
			}
			if (orig.max.y > clip.max.y) {
				uv.max.y -= (orig.max.y - clip.max.y) * (uvsize.y / (orig.max.y - orig.min.y));
				orig.max.y = clip.max.y;
			}
			if (orig.max.y <= orig.min.y) { continue; }

			addVertex(orig.min, uv.min);
			addVertex({orig.min.x, orig.max.y}, uv.min + glm::vec2{0, uv.max.y});
			addVertex(orig.max, uv.min + uv.max);
			addVertex({orig.max.x, orig.min.y}, uv.min + glm::vec2{uv.max.x, 0});

			addElements(base, base+1, base+2);
			addElements(base+2, base+3, base);
			base += 4;
		}

		setTexture(old);
		return pos - drawOffset;
	}
};
