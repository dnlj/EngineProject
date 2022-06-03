// Engine
#include <Engine/Gui/DrawBuilder.hpp>


namespace Engine::Gui {
	DrawBuilder::DrawBuilder(Gfx::ShaderLoader& shaderLoader, TextureManager& textureManager) {
		polyShader = shaderLoader.get("shaders/gui_poly");
		glyphShader = shaderLoader.get("shaders/gui_glyph");

		glProgramUniform1i(glyphShader->get(), 1, 0);
		glProgramUniform1i(polyShader->get(), 1, 0);

		{
			constexpr static GLuint bindingIndex = 0;
			GLuint attribLocation = -1;

			glCreateVertexArrays(1, &polyVAO);

			glCreateBuffers(1, &polyEBO);
			glVertexArrayElementBuffer(polyVAO, polyEBO);

			glCreateBuffers(1, &polyVBO);
			glVertexArrayVertexBuffer(polyVAO, bindingIndex, polyVBO, 0, sizeof(PolyVertex));

			glEnableVertexArrayAttrib(polyVAO, ++attribLocation);
			glVertexArrayAttribBinding(polyVAO, attribLocation, bindingIndex);
			glVertexArrayAttribFormat(polyVAO, attribLocation, 2, GL_FLOAT, GL_FALSE, offsetof(PolyVertex, pos));

			glEnableVertexArrayAttrib(polyVAO, ++attribLocation);
			glVertexArrayAttribBinding(polyVAO, attribLocation, bindingIndex);
			glVertexArrayAttribFormat(polyVAO, attribLocation, 2, GL_UNSIGNED_SHORT, GL_TRUE, offsetof(PolyVertex, texCoord));

			glEnableVertexArrayAttrib(polyVAO, ++attribLocation);
			glVertexArrayAttribBinding(polyVAO, attribLocation, bindingIndex);
			glVertexArrayAttribFormat(polyVAO, attribLocation, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(PolyVertex, color));
		}

		{
			constexpr static GLuint bindingIndex = 0;
			GLuint attribLocation = -1;

			glCreateBuffers(1, &glyphVBO);

			glCreateVertexArrays(1, &glyphVAO);
			glVertexArrayVertexBuffer(glyphVAO, bindingIndex, glyphVBO, 0, sizeof(GlyphVertex));

			glEnableVertexArrayAttrib(glyphVAO, ++attribLocation);
			glVertexArrayAttribBinding(glyphVAO, attribLocation, bindingIndex);
			glVertexArrayAttribFormat(glyphVAO, attribLocation, 2, GL_FLOAT, GL_FALSE, offsetof(GlyphVertex, pos));

			glEnableVertexArrayAttrib(glyphVAO, ++attribLocation);
			glVertexArrayAttribBinding(glyphVAO, attribLocation, bindingIndex);
			glVertexArrayAttribFormat(glyphVAO, attribLocation, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(GlyphVertex, color));

			glEnableVertexArrayAttrib(glyphVAO, ++attribLocation);
			glVertexArrayAttribBinding(glyphVAO, attribLocation, bindingIndex);
			glVertexArrayAttribIFormat(glyphVAO, attribLocation, 1, GL_UNSIGNED_INT, offsetof(GlyphVertex, index));
		}

		{
			Image img{PixelFormat::RGB8, {1,1}};
			memset(img.data(), 0xFF, img.sizeBytes());
			defaultTexture.setStorage(TextureFormat::RGB8, img.size());
			defaultTexture.setImage(img);
		}

		activeTexture = defaultTexture;
		reset();
	}

	DrawBuilder::~DrawBuilder() {
		glDeleteVertexArrays(1, &polyVAO);
		glDeleteBuffers(1, &polyEBO);
		glDeleteBuffers(1, &polyVBO);

		glDeleteVertexArrays(1, &glyphVAO);
		glDeleteBuffers(1, &glyphVBO);
	}

	void DrawBuilder::resize(glm::vec2 viewport) {
		view = viewport;
		const auto view2 = 2.0f / view;
		glProgramUniform2fv(polyShader->get(), 0, 1, &view2.x);
		glProgramUniform2fv(glyphShader->get(), 0, 1, &view2.x);
	}

	void DrawBuilder::finish() {
		// Needed to populate count of last draw groups
		if (auto last = polyDrawGroups.rbegin(); last != polyDrawGroups.rend()) {
			last->count = static_cast<int32>(polyElementData.size()) - last->offset;
		}

		if (auto last = glyphDrawGroups.rbegin(); last != glyphDrawGroups.rend()) {
			last->count = static_cast<int32>(glyphVertexData.size()) - last->offset;
		}

		ENGINE_DEBUG_ASSERT(clipStack.size() == 1, "Mismatched push/pop clip");
	}

	void DrawBuilder::draw() {
		// Update font buffers
		fontManager2.updateAllFontDataBuffers();

		// Update poly vertex/element buffer
		if (const auto count = polyVertexData.size()) {
			{
				const auto size = count * sizeof(polyVertexData[0]);
				if (size > polyVBOCapacity) {
					polyVBOCapacity = static_cast<GLsizei>(polyVertexData.capacity() * sizeof(polyVertexData[0]));
					glNamedBufferData(polyVBO, polyVBOCapacity, nullptr, GL_DYNAMIC_DRAW);
				}
				glNamedBufferSubData(polyVBO, 0, size, polyVertexData.data());
			}
			{
				const auto size = polyElementData.size() * sizeof(polyElementData[0]);
				if (size > polyEBOCapacity) {
					polyEBOCapacity = static_cast<GLsizei>(polyElementData.capacity() * sizeof(polyElementData[0]));
					glNamedBufferData(polyEBO, polyEBOCapacity, nullptr, GL_DYNAMIC_DRAW);
				}
				glNamedBufferSubData(polyEBO, 0, size, polyElementData.data());
			}
		}

		// Update glyph vertex buffer
		if (const auto count = glyphVertexData.size()) {
			const GLsizei newSize = static_cast<GLsizei>(count * sizeof(GlyphVertex));
			if (newSize > glyphVBOCapacity) {
				glyphVBOCapacity = static_cast<GLsizei>(glyphVertexData.capacity() * sizeof(glyphVertexData[0]));
				glNamedBufferData(glyphVBO, glyphVBOCapacity, nullptr, GL_DYNAMIC_DRAW);
			}
			glNamedBufferSubData(glyphVBO, 0, newSize, glyphVertexData.data());
		}

		const auto scissor = [](const Bounds& bounds, float32 y) ENGINE_INLINE {
			glScissor(
				static_cast<int32>(bounds.min.x),
				static_cast<int32>(y - bounds.max.y),
				static_cast<int32>(bounds.getWidth()),
				static_cast<int32>(bounds.getHeight())
			);
		};

		auto currPolyGroup = polyDrawGroups.begin();
		const auto lastPolyGroup = polyDrawGroups.end();
		auto currGlyphGroup = glyphDrawGroups.begin();
		const auto lastGlyphGroup = glyphDrawGroups.end();

		for (int32 z = 0; z <= zindex;) {
			// Draw polys
			if (currPolyGroup != lastPolyGroup && currPolyGroup->zindex == z) {
				glBindVertexArray(polyVAO);
				glUseProgram(polyShader->get());
				TextureHandle2D activeTex = {};

				do {
					ENGINE_DEBUG_ASSERT(currPolyGroup->count != 0, "Empty draw group. This group should have been skipped/removed already.");

					if (currPolyGroup->tex != activeTex) {
						activeTex = currPolyGroup->tex;
						glBindTextureUnit(0, activeTex.get());
					}

					scissor(currPolyGroup->clip, view.y);
					glDrawElements(GL_TRIANGLES,
						currPolyGroup->count,
						sizeof(polyElementData[0]) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
						(void*)(currPolyGroup->offset * sizeof(polyElementData[0]))
					);

					++currPolyGroup;
					++z;
				} while (currPolyGroup != lastPolyGroup && currPolyGroup->zindex == z);
			}

			// Draw glyphs
			if (currGlyphGroup != lastGlyphGroup && currGlyphGroup->zindex == z) {
				glBindVertexArray(glyphVAO);
				glUseProgram(glyphShader->get());
				Font activeFont = nullptr;

				do {
					ENGINE_DEBUG_ASSERT(currGlyphGroup->count != 0, "Empty draw group. This group should have been skipped/removed already.");
					ENGINE_DEBUG_ASSERT(currGlyphGroup->font != nullptr);

					if (currGlyphGroup->font != activeFont) {
						activeFont = currGlyphGroup->font;
						glBindTextureUnit(0, activeFont->getGlyphTexture().get());
						glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, activeFont->getGlyphDataBuffer());
					}

					scissor(currGlyphGroup->clip, view.y);
					glDrawArrays(GL_POINTS, currGlyphGroup->offset, currGlyphGroup->count);

					++currGlyphGroup;
					++z;
				} while (currGlyphGroup != lastGlyphGroup && currGlyphGroup->zindex == z);
			}
		}
	}

	void DrawBuilder::reset() {
		clipStack.clear();
		clipStack.push_back({{0,0}, view});

		zindex = -1;

		polyElementData.clear();
		polyVertexData.clear();
		polyDrawGroups.clear();

		glyphVertexData.clear();
		glyphDrawGroups.clear();
	}

	void DrawBuilder::nextDrawGroupPoly() {
		if (polyDrawGroups.empty()) {
			++zindex;
			polyDrawGroups.emplace_back();
		}

		const auto sz = static_cast<int32>(polyElementData.size());
		auto& prev = polyDrawGroups.back();
		prev.count = sz - prev.offset;

		const auto setup = [&](PolyDrawGroup& group) ENGINE_INLINE {
			group.zindex = zindex;
			group.offset = sz;
			group.clip = clipStack.back();
			group.tex = activeTexture;
		};
		
		// Figure out if we need a new group, can reuse an empty group, or extend the previous group
		if (prev.count == 0) {
			setup(prev);
		} else if (prev.zindex != zindex
			|| prev.clip != clipStack.back()
			|| prev.tex != activeTexture) {
			++zindex;
			setup(polyDrawGroups.emplace_back());
		} else {
			// No group change needed, extend the previous group
		}
	}
	
	void DrawBuilder::nextDrawGroupGlyph() {
		if (glyphDrawGroups.empty()) {
			++zindex;
			glyphDrawGroups.emplace_back();
		}

		const auto sz = static_cast<int32>(glyphVertexData.size());
		auto& prev = glyphDrawGroups.back();
		prev.count = sz - prev.offset;

		const auto setup = [&](GlyphDrawGroup& group) ENGINE_INLINE {
			group.zindex = zindex;
			group.offset = sz;
			group.clip = clipStack.back();
			group.font = font;
			ENGINE_DEBUG_ASSERT(font != nullptr);
		};

		// Figure out if we need a new group, can reuse an empty group, or extend the previous group
		if (prev.count == 0) {
			setup(prev);
		} else if (prev.zindex != zindex
			|| prev.font != font
			|| prev.clip != clipStack.back()) {
			++zindex;
			setup(glyphDrawGroups.emplace_back());
		} else {
			// No group change needed, extend the previous group
		}
	}

	void DrawBuilder::pushClip() {
		clipStack.push_back(clipStack.back());
	}
			
	void DrawBuilder::popClip() {
		ENGINE_DEBUG_ASSERT(!clipStack.empty(), "Attempting to pop empty clipping stack");
		clipStack.pop_back();
	}

	void DrawBuilder::setClip(Bounds bounds) {
		auto& curr = clipStack.back();
		curr = (clipStack.end() - 2)->intersect(bounds);
		curr.max = glm::max(curr.min, curr.max);
	}

	void DrawBuilder::drawTexture(TextureHandle2D tex, glm::vec2 pos, glm::vec2 size) {
		const auto old = activeTexture;
		activeTexture = tex;
		drawRect(pos, size, {1,1,1,1});
		activeTexture = old;
	}

	void DrawBuilder::drawPoly(ArrayView<const glm::vec2> points, glm::vec4 color) {
		ENGINE_DEBUG_ASSERT(points.size() >= 3, "Must have at least three points");
		nextDrawGroupPoly();

		const auto base = static_cast<int32>(polyVertexData.size());
		const auto psz = points.size();
		drawVertex(points[0], color);
		drawVertex(points[1], color);

		for (int i = 2; i < psz; ++i) {
			drawVertex(points[i], color);
			addPolyElements(base, base + i - 1, base + i);
		}
	}

	void DrawBuilder::drawRect(glm::vec2 pos, glm::vec2 size, glm::vec4 color) {
		nextDrawGroupPoly();
		auto base = static_cast<uint32>(polyVertexData.size());
		drawVertex(pos, {0,1}, color);
		drawVertex(pos + glm::vec2{0, size.y}, {0,0}, color);
		drawVertex(pos + size, {1,0}, color);
		drawVertex(pos + glm::vec2{size.x, 0}, {1,1}, color);
		addPolyElements(base, base+1, base+2);
		addPolyElements(base+2, base+3, base);
	}
	
	void DrawBuilder::drawLine(glm::vec2 a, glm::vec2 b, float32 width, glm::vec4 color) {
		const auto t = glm::normalize(b - a);
		const auto n = width * 0.5f * glm::vec2{-t.y, t.x};
		drawPoly({a - n, a + n, b + n, b - n}, color);
	}

	void DrawBuilder::drawString(glm::vec2 pos, glm::vec4 color, Font font, ArrayView<const ShapeGlyph> glyphs) {
		ENGINE_DEBUG_ASSERT(font != nullptr, "Attempting to draw string with null font.");
		if (glyphs.empty()) { return; }

		this->font = font;
		pos += drawOffset;
		nextDrawGroupGlyph();

		for (const auto& data : glyphs) {
			const uint32 index = font->getGlyphIndex(data.index);
			glyphVertexData.push_back({
				.pos = glm::round(pos + data.offset),
				.color = color * 255.0f,
				.index = index,
			});
			pos += data.advance;
		}
	}
};
