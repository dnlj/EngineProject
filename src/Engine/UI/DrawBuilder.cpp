// Engine
#include <Engine/UI/DrawBuilder.hpp>
#include <Engine/Gfx/ShaderManager.hpp>


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
			
			glEnableVertexArrayAttrib(polyVAO, ++attribLocation);
			glVertexArrayAttribBinding(polyVAO, attribLocation, bindingIndex);
			glVertexArrayAttribFormat(polyVAO, attribLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, offsetof(PolyVertex, layer));
		}

		{
			using namespace Gfx;

			// TODO: rm
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
	}

	void DrawBuilder::resize(glm::vec2 viewport) {
		view = viewport;
		const auto view2 = 2.0f / view;
		glProgramUniform2fv(polyShader->get(), 0, 1, &view2.x);
	}

	void DrawBuilder::finish() {
		// Needed to populate count of last draw groups
		if (auto last = polyDrawGroups.rbegin(); last != polyDrawGroups.rend()) {
			last->count = static_cast<int32>(polyElementData.size()) - last->offset;
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

		const auto scissor = [y=view.y](const Bounds& bounds) ENGINE_INLINE {
			glScissor(
			      static_cast<int32>(bounds.min.x),
			      static_cast<int32>(y - bounds.max.y),
			      static_cast<int32>(bounds.getWidth()),
			      static_cast<int32>(bounds.getHeight())
			);
		};

		// Draw polys
		for (auto const& group : polyDrawGroups) {
			// TODO: ENGINE_DEBUG_ASSERT(group.count > 0);

			glBindVertexArray(polyVAO);
			glUseProgram(polyShader->get());
			Gfx::TextureHandleGeneric activeTex = {};

			if (!group.clip.empty()) {
				scissor(group.clip);
			} else {
				// TODO: exclude redundant calls
				scissor(clipStack.front());
			}

			if (group.tex != activeTex) {
				activeTex = group.tex;
				glBindTextureUnit(0, activeTex.get());
			}

			glDrawElements(GL_TRIANGLES,
				group.count,
				sizeof(polyElementData[0]) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
				(void*)(group.offset * sizeof(polyElementData[0]))
			);
		}
	}

	void DrawBuilder::reset() {
		clipStack.clear();
		clipStack.push_back({{0,0}, view});
		lastClipOffset = 0;

		polyElementData.clear();
		polyVertexData.clear();
		polyDrawGroups.clear();
	}

	void DrawBuilder::nextDrawGroupPoly() {
		if (polyDrawGroups.empty()) { // TODO: probably just make sure we push one in reset instead of having a check here.
			polyDrawGroups.emplace_back();
		}

		const auto sz = static_cast<int32>(polyElementData.size());
		auto& prev = polyDrawGroups.back();
		prev.count = sz - prev.offset;
		ENGINE_DEBUG_ASSERT(prev.count >= 0); // TODO: this should just be > zero not >=. Why do we fail this?

		const auto setup = [&](PolyDrawGroup& group) ENGINE_INLINE {
			group.offset = sz;
			group.tex = activeTexture;
			lastClipOffset = group.offset;
		};
		
		// Figure out if we need a new group, can reuse an empty group, or extend the previous group
		if (prev.count == 0) {
			setup(prev);
		} else if (false
			|| (prev.tex != activeTexture)
			//|| (!prev.clip.empty() && prev.clip != clipStack.back())
			) {
			setup(polyDrawGroups.emplace_back());
		} else {
			// No group change needed, extend the previous group
		}
	}

	void DrawBuilder::makeHardwareClip() {
		auto* last = &polyDrawGroups.back();
		if (!last->clip.empty()) { return; }

		// We need to split the previous group
		if (last->offset != lastClipOffset) {
			ENGINE_DEBUG_ASSERT(lastClipOffset > last->offset);
			auto* next = &polyDrawGroups.emplace_back(*last);
			next->offset = lastClipOffset;
			last->count = next->offset - last->offset;
			last = next;
		}

		last->clip = clipStack.back();
	}

	void DrawBuilder::pushClip() {
		clipStack.push_back(clipStack.back());
		//
		//
		// TODO: really should just change this function to take a Bounds and remove setClip? With error checking for to make sure we match push/pop calls or anobject with destructor
		//       Maybe some kind of ClipScope{}; object?
		// TODO: check clip != last clip, if it is we should be able to just keep the prev lastClipOffset
		//       We cant do that yet though because we have setClip still
		//
		//
		//
		lastClipOffset = static_cast<int32>(polyElementData.size());
	}
			
	void DrawBuilder::popClip() {
		ENGINE_DEBUG_ASSERT(!clipStack.empty(), "Attempting to pop empty clipping stack");
		clipStack.pop_back();

		// TODO: again check that back != old back, if is just continue the prev offset
		lastClipOffset = static_cast<int32>(polyElementData.size());
	}

	void DrawBuilder::setClip(Bounds bounds) {
		auto& curr = clipStack.back();
		curr = (clipStack.end() - 2)->intersect(bounds);
		curr.max = glm::max(curr.min, curr.max);
	}

	void DrawBuilder::drawTexture(Gfx::TextureHandle2D tex, glm::vec2 pos, glm::vec2 size) {
		const auto old = activeTexture;
		setTexture(tex);
		drawRect(pos, size);
		setTexture(old);
	}

	void DrawBuilder::drawPoly(ArrayView<const glm::vec2> points) {
		ENGINE_DEBUG_ASSERT(points.size() >= 3, "Must have at least three points");
		nextDrawGroupPoly();
		makeHardwareClip();

		const auto base = static_cast<int32>(polyVertexData.size());
		const auto psz = points.size();
		drawVertex(points[0]);
		drawVertex(points[1]);

		for (int i = 2; i < psz; ++i) {
			drawVertex(points[i]);
			addPolyElements(base, base + i - 1, base + i);
		}
	}

	void DrawBuilder::drawRect(glm::vec2 pos, glm::vec2 size) {
		const auto pvdSz = polyVertexData.size();
		auto base = static_cast<uint32>(pvdSz);

		pos += drawOffset;

		ENGINE_DEBUG_ASSERT(clipStack.size() > 0);
		const auto rect = clipStack.back().intersect({pos, pos+size});
		if (rect.max.x <= rect.min.x || rect.max.y <= rect.min.y) { return; }

		nextDrawGroupPoly();
		drawVertex2(rect.min, {0,1});
		drawVertex2({rect.min.x, rect.max.y}, {0,0});
		drawVertex2(rect.max, {1,0});
		drawVertex2({rect.max.x, rect.min.y}, {1,1});

		addPolyElements(base, base+1, base+2);
		addPolyElements(base+2, base+3, base);
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
		const auto old = activeTexture;
		auto base = static_cast<uint32>(polyVertexData.size()); // TODO: should be able to just check once then do += 4
		setTexture(font->getGlyphTexture());

		for (const auto& data : glyphs) ENGINE_INLINE_CALLS {
			const uint32 index = font->getGlyphIndex(data.index);
			const glm::vec2 offset = glyphData[index].offset / 4096.0f;
			auto size = glyphData[index].size;
			const auto uvsize = size / 4096.0f;

			const auto p = glm::round(pos + data.offset + drawOffset); // I think this should technically also include the size offset per vert. In practice it does not make a difference (in any tests i have done) and this is faster.
			pos += data.advance;
			ENGINE_DEBUG_ASSERT(clipStack.size() > 0);
			const auto& clip = clipStack.back();
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


			int layer = 0; // TODO:
			drawVertex2(orig.min, uv.min, layer);
			drawVertex2({orig.min.x, orig.max.y}, uv.min + glm::vec2{0, uv.max.y}, layer);
			drawVertex2(orig.max, uv.min + uv.max, layer);
			drawVertex2({orig.max.x, orig.min.y}, uv.min + glm::vec2{uv.max.x, 0}, layer);

			addPolyElements(base, base+1, base+2);
			addPolyElements(base+2, base+3, base);
			base += 4;
		}

		setTexture(old);
		return pos - drawOffset;
	}
};
