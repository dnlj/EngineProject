// FreeType
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>

// Engine
#include <Engine/Gui/Context.hpp>


namespace Engine::Gui {
	Context::Context(Engine::EngineInstance& engine) {
		shader = engine.shaderManager.get("shaders/gui_clip");
		textShader = engine.shaderManager.get("shaders/gui_text");
		view = engine.camera.getScreenSize(); // TODO: should update when resized

		{
			constexpr float32 mscale = 1.0f/64;
			FT_Library ftlib;
			if (const auto err = FT_Init_FreeType(&ftlib)) {
				ENGINE_ERROR("FreeType error: ", err); // TODO: actual error
			}

			FT_Face face;
			if (const auto err = FT_New_Face(ftlib, "assets/arial.ttf", 0, &face)) {
				ENGINE_ERROR("FreeType error: ", err); // TODO: actual error
			}

			if (const auto err = FT_Set_Pixel_Sizes(face, 0, 96)) {
				ENGINE_ERROR("FreeType error: ", err); // TODO: actual error
			}

			ENGINE_LOG("FaceX: ", face->bbox.xMax - face->bbox.xMin);
			ENGINE_LOG("FaceY: ", face->bbox.yMax - face->bbox.yMin);
			ENGINE_LOG("FaceXPx: ", FT_MulFix(face->bbox.xMax - face->bbox.xMin, face->size->metrics.x_scale) * (1.0/64));
			ENGINE_LOG("FaceYPx: ", FT_MulFix(face->bbox.yMax - face->bbox.yMin, face->size->metrics.y_scale) * (1.0/64));
			ENGINE_LOG("UPM: ", face->units_per_EM);

			// In practice most glyphs are much smaller than `face->bbox` (every ascii char is < 1/2 the size depending on font)
			// It would be better to use a packing algorithm, but then we would want
			// to periodically repack (glCopy*, not reload) as glyphs are unloaded which complicates things.
			// Still very doable. Just requires more work.
			{
				// TODO:
				GLint texSize;
				glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);
				ENGINE_INFO("Maximum texture size: ", texSize);

				// TODO: may want to limit base on face size? for small font sizes 4096 might be excessive.
				texSize = std::min(texSize, 4096);

				// TODO: i think this may be subtly incorrect.
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
				const glm::vec2 maxFace = glm::ceil(glm::vec2{
					FT_MulFix(face->bbox.xMax - face->bbox.xMin, face->size->metrics.x_scale) * mscale,
					FT_MulFix(face->bbox.yMax - face->bbox.yMin, face->size->metrics.y_scale) * mscale
				});

				glm::vec2 maxGlyph = {};

				const auto startT = Clock::now();
				for (int i = 0; FT_Load_Glyph(face, i, FT_LOAD_DEFAULT) == 0; ++i) {
					FT_Glyph glyph;
					FT_BBox bbox;
					if (FT_Get_Glyph(face->glyph, &glyph)) { ENGINE_WARN("Oh no!"); break; }
					FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_GRIDFIT, &bbox);
					FT_Done_Glyph(glyph);
					maxGlyph.x = std::max(maxGlyph.x, (bbox.xMax - bbox.xMin) * mscale);
					maxGlyph.y = std::max(maxGlyph.y, (bbox.yMax - bbox.yMin) * mscale);
				}
				const auto endT = Clock::now();
				ENGINE_LOG("Time: ", Clock::Milliseconds{endT-startT}.count());
				ENGINE_LOG("Max Glyph: ", maxGlyph.x, ", ", maxGlyph.y);
				ENGINE_LOG("Max Face: ", maxFace.x, ", ", maxFace.y);
			}

			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			for (uint8 c = ' '; c <= '~'; ++c) {
				if (const auto err = FT_Load_Char(face, c, FT_LOAD_RENDER)) {
					ENGINE_ERROR("FreeType error: ", err); // TODO: actual error
				}
				const auto& glyph = *face->glyph;
				const auto& metrics = glyph.metrics;

				// We currently only support horizontal bearing/advance
				CharData& data = charDataMap[c];
				data.size = {metrics.width * mscale, metrics.height * mscale};
				data.bearing = {metrics.horiBearingX * mscale, metrics.horiBearingY * -mscale};
				data.advance = metrics.horiAdvance * mscale;
				if (glyph.bitmap.width) {
					data.tex.setStorage(TextureFormat::R8, data.size);
					data.tex.setSubImage(0, {}, data.size, PixelFormat::R8, glyph.bitmap.buffer);
				}
			}
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

			if (const auto err = FT_Done_Face(face)) {
				ENGINE_ERROR("FreeType error: ", err); // TODO: actual error
			}
			
			if (const auto err = FT_Done_FreeType(ftlib)) {
				ENGINE_ERROR("FreeType error: ", err); // TODO: actual error
			}
			
			glCreateBuffers(1, &textVBO);
			glNamedBufferData(textVBO, 6 * 4 * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);

			glCreateVertexArrays(1, &textVAO);
			glVertexArrayVertexBuffer(textVAO, 0, textVBO, 0, 4 * sizeof(GLfloat));

			glEnableVertexArrayAttrib(textVAO, 0);
			glVertexArrayAttribBinding(textVAO, 0, 0);
			glVertexArrayAttribFormat(textVAO, 0, 2, GL_FLOAT, GL_FALSE, 0);

			glEnableVertexArrayAttrib(textVAO, 1);
			glVertexArrayAttribBinding(textVAO, 1, 0);
			glVertexArrayAttribFormat(textVAO, 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat));
		}

		{
			quadShader = engine.shaderManager.get("shaders/fullscreen_passthrough");
			const glm::vec2 quadData[] = {
				{1,1}, {-1,1}, {-1,-1},
				{-1,-1}, {1,-1}, {1,1},
			};

			// TODO: passthrough prgoram
			glCreateBuffers(1, &quadVBO);
			glNamedBufferData(quadVBO, sizeof(quadData), &quadData, GL_STATIC_DRAW);

			glCreateVertexArrays(1, &quadVAO);
			glVertexArrayVertexBuffer(quadVAO, 0, quadVBO, 0, sizeof(quadData[0]));
			glEnableVertexArrayAttrib(quadVAO, 0);
			glVertexArrayAttribBinding(quadVAO, 0, 0);
			glVertexArrayAttribFormat(quadVAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
		}

		glCreateFramebuffers(1, &fbo);

		glCreateVertexArrays(1, &vao);
		glCreateBuffers(1, &vbo);
		glVertexArrayVertexBuffer(vao, vertBindingIndex, vbo, 0, sizeof(Vertex));

		// Vertex
		glEnableVertexArrayAttrib(vao, 0);
		glVertexArrayAttribBinding(vao, 0, vertBindingIndex);
		glVertexArrayAttribFormat(vao, 0, 4, GL_FLOAT, GL_FALSE, offsetof(Vertex, color));

		glEnableVertexArrayAttrib(vao, 1);
		glVertexArrayAttribBinding(vao, 1, vertBindingIndex);
		glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));

		glEnableVertexArrayAttrib(vao, 2);
		glVertexArrayAttribBinding(vao, 2, vertBindingIndex);
		glVertexArrayAttribFormat(vao, 2, 1, GL_FLOAT, GL_FALSE, offsetof(Vertex, id));
		//glVertexArrayAttribIFormat(vao, 2, 1, GL_UNSIGNED_INT, offsetof(Vertex, id));
		
		glEnableVertexArrayAttrib(vao, 3);
		glVertexArrayAttribBinding(vao, 3, vertBindingIndex);
		glVertexArrayAttribFormat(vao, 3, 1, GL_FLOAT, GL_FALSE, offsetof(Vertex, pid));
		//glVertexArrayAttribIFormat(vao, 3, 1, GL_UNSIGNED_INT, offsetof(Vertex, pid));

		registerPanel(nullptr); // register before everything else so nullptr = id 0

		root = new Panel{};
		root->setPos({25, 50});
		root->setSize({512, 256});
		registerPanel(root);

		{
			auto child = root->addChild(new Panel{});
			child->setPos({0, 0});
			child->setSize({64, 300});
			registerPanel(child);

			auto childChild = child->addChild(new Panel{});
			childChild->setPos({0, 0});
			childChild->setSize({32, 32});
			registerPanel(childChild);
		}
		
		{
			auto child = root->addChild(new Panel{});
			child->setPos({128, 5});
			child->setSize({32, 64});
			registerPanel(child);
		}
	}

	Context::~Context() {
		glDeleteVertexArrays(1, &quadVAO);
		glDeleteBuffers(1, &quadVBO);

		glDeleteFramebuffers(1, &fbo);
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);

		glDeleteVertexArrays(1, &textVAO);
		glDeleteBuffers(1, &textVBO);

		delete root;
	}

	void Context::renderText(const std::string_view str) {
		struct Vert {
			glm::vec2 pos;
			glm::vec2 texCoord;
		} verts[6] = {
			{{0.0f, 0.0f}, {0.0f, 1.0f}},
			{{0.0f, 1.0f}, {0.0f, 0.0f}},
			{{1.0f, 0.0f}, {1.0f, 1.0f}},

			{{0.0f, 1.0f}, {0.0f, 0.0f}},
			{{1.0f, 1.0f}, {1.0f, 0.0f}},
			{{1.0f, 0.0f}, {1.0f, 1.0f}},
		};

		glm::vec2 base = {64, 512};

		glBindVertexArray(textVAO);
		glUseProgram(textShader->get());
		glUniform2fv(0, 1, &view.x);
		glUniform1i(1, 0);

		for (const uint8 c : str) {
			const auto& data = charDataMap[c];

			if (data.tex.get()) {
				auto p = base + data.bearing;

				verts[0] = {{p.x, p.y}, {0.0f, 0.0f}};
				verts[1] = {{p.x, p.y + data.size.y}, {0.0f, 1.0f}};
				verts[2] = {{p.x + data.size.x, p.y}, {1.0f, 0.0f}};
				verts[3] = {{p.x, p.y + data.size.y}, {0.0f, 1.0f}};
				verts[4] = {{p.x + data.size.x, p.y + data.size.y}, {1.0f, 1.0f}};
				verts[5] = {{p.x + data.size.x, p.y}, {1.0f, 0.0f}};

				glNamedBufferSubData(textVBO, 0, sizeof(verts), &verts[0]);
				glBindTextureUnit(0, data.tex.get());
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}

			// Assume we want a horizontal layout
			base.x += data.advance;
		}
	}

	void Context::render() {
		if (!hoverValid) {
			updateHover();
			hoverValid = true;
		}

		const Panel* curr = root;
		offset = {};
		multiDrawData.first.emplace_back() = 0;
		
		// Breadth first traversal
		while (true) {
			while (curr) {
				if (curr->firstChild) {
					bfsNext.emplace_back(
						offset + curr->getPos(),
						curr->firstChild
					);
				}

				if (false) {}
				else if (curr == getActive()) { currRenderState.color = glm::vec4{1, 0, 1, 0.2}; }
				else if (curr == getHover()) { currRenderState.color = glm::vec4{1, 1, 0, 0.2}; }
				else { currRenderState.color = glm::vec4{1, 0, 0, 0.2}; }

				currRenderState.current = curr;
				curr->render(*this);
				curr = curr->nextSibling;
			}

			if (bfsCurr.empty()) {
				const auto vsz = static_cast<GLint>(verts.size());
				multiDrawData.count.emplace_back() = vsz - multiDrawData.first.back();

				bfsCurr.swap(bfsNext);
				if (bfsCurr.empty()) { break; }

				multiDrawData.first.emplace_back() = vsz;
			}

			const auto& back = bfsCurr.back();
			curr = back.panel;
			offset = back.offset;
			bfsCurr.pop_back();
		}

		{
			const auto size = verts.size() * sizeof(verts[0]);
			if (size > vboCapacity) {
				vboCapacity = static_cast<GLsizei>(verts.capacity() * sizeof(verts[0]));
				glNamedBufferData(vbo, vboCapacity, nullptr, GL_DYNAMIC_DRAW);
			}
			glNamedBufferSubData(vbo, 0, size, verts.data());
		}

		glBindVertexArray(vao);
		glUseProgram(shader->get());
		glUniform2fv(0, 1, &view.x);

		glEnable(GL_BLEND);
		glBlendFuncSeparatei(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glBlendFunci(1, GL_ONE, GL_ZERO);

		// If we dont care about clipping we can just do
		//glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(verts.size()));

		glClearTexImage(colorTex.get(), 0, GL_RGB, GL_FLOAT, 0);
		glClearTexImage(clipTex1.get(), 0, GL_RGB, GL_FLOAT, 0);
		glClearTexImage(clipTex2.get(), 0, GL_RGB, GL_FLOAT, 0);

		// We cant use glMultiDrawArrays because you can not read/write
		// the same texture. It may be possible to work around this
		// with glTextureBarrier but that isnt as widely supported.
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		for (int i = 0; i < multiDrawData.first.size(); ++i) {
			const auto first = multiDrawData.first[i];
			const auto count = multiDrawData.count[i];

			const GLenum buffs[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 + activeClipTex};
			glNamedFramebufferDrawBuffers(fbo, 2, &buffs[0]);
			glBindTextureUnit(0, activeClipTex ? clipTex1.get() : clipTex2.get());
			glDrawArrays(GL_TRIANGLES, first, count);
			activeClipTex = !activeClipTex;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glBindVertexArray(quadVAO);
		glUseProgram(quadShader->get());
		glBindTextureUnit(0, colorTex.get());
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisable(GL_BLEND);
		verts.clear();
		multiDrawData.first.clear();
		multiDrawData.count.clear();

		renderText("Hello, world!");
	}

	void Context::addRect(const glm::vec2 pos, const glm::vec2 size) {
		const PanelId id = getPanelId(currRenderState.current);
		const PanelId pid = getPanelId(currRenderState.current->parent);
		const auto color = currRenderState.color;

		verts.push_back({.color = color, .pos = offset + pos, .id = id, .pid = pid});
		verts.push_back({.color = color, .pos = offset + pos + glm::vec2{0, size.y}, .id = id, .pid = pid});
		verts.push_back({.color = color, .pos = offset + pos + size, .id = id, .pid = pid});

		verts.push_back({.color = color, .pos = offset + pos + size, .id = id, .pid = pid});
		verts.push_back({.color = color, .pos = offset + pos + glm::vec2{size.x, 0}, .id = id, .pid = pid});
		verts.push_back({.color = color, .pos = offset + pos, .id = id, .pid = pid});
	}

	void Context::updateHover() {
		Panel* const old = getHover();
		glm::vec2 off = {};
		Panel* curr = nullptr;

		// Rebuild offset and find where our old stack ends
		{
			auto it = hoverStack.begin();
			auto end = hoverStack.end();

			for (; it != end; ++it) {
				auto* panel = *it;

				if (panel->canHover() && panel->parent == curr && (panel->getBounds() + off).contains(cursor)) {
					off += panel->getPos();
					curr = panel;
				} else {
					break;
				}
			}

			if (it != end) {
				const auto start = it;
				auto next = it + 1;
				while (next != end) {
					(*it)->onEndChildHover(*next);
					it = next;
					++next;
				}

				hoverStack.erase(start, end);
			}
		}

		// Setup the next panel to check
		curr = curr ? curr->firstChild : root;

		while (curr) {
			if (curr->canHover() && (curr->getBounds() + off).contains(cursor)) {
				if (curr->parent && curr->parent->onBeginChildHover(curr)) {
					break;
				}

				off += curr->getPos();
				hoverStack.push_back(curr);
				curr = curr->firstChild;
			} else {
				curr = curr->nextSibling;
			}
		}

		Panel* target = getHover();
		if (target != old) {
			if (old) { old->onEndHover(); };
			if (target) { target->onBeginHover(); }
		}
	}

	bool Context::onMouse(const Engine::Input::InputEvent event) {
		//ENGINE_LOG("onMouse:",
		//	" ", event.state.value,
		//	" ", (int)event.state.id.code,
		//	" ", (int)event.state.id.type,
		//	" ", (int)event.state.id.device,
		//	" @ ", Engine::Clock::Seconds{event.time.time_since_epoch()}.count()
		//);
		if (event.state.id.code == 0) {
			if (event.state.value && isHoverAny()) {
				ENGINE_DEBUG_ASSERT(active == nullptr);
				auto focus = getFocus();
				if (!focus) { return false; }
				focus->onBeginActivate();
				active = focus;
				return true;
			} else if (active) {
				active->onEndActivate();
				active = nullptr;
				return true;
			}
		}
		return false;
	}

	bool Context::onMouseMove(const Engine::Input::InputEvent event) {
		//ENGINE_LOG("onMouseMove:", " ", event.state.id.code, " ", event.state.valuef, " @ ", Engine::Clock::Seconds{event.time.time_since_epoch()}.count());
		if (event.state.id.code == 0) {
			cursor.x = event.state.valuef;
		} else {
			cursor.y = event.state.valuef;
		}
		hoverValid = false;
		return isHoverAny();
	}

	bool Context::onMouseWheel(const Engine::Input::InputEvent event) {
		// ENGINE_LOG("onMouseWheel: ", event.state.value, " @ ", Engine::Clock::Seconds{event.time.time_since_epoch()}.count());
		return false;
	}

	bool Context::onKey(const Engine::Input::InputEvent event) {
		// ENGINE_LOG("onKey: ", event.state.value, " @ ", Engine::Clock::Seconds{event.time.time_since_epoch()}.count());
		return false;
	}

	bool Context::onChar(const wchar_t ch) {
		// ENGINE_LOG("onChar: ", (int)ch);
		return false;
	}

	void Context::onResize(const int32 w, const int32 h) {
		if (w == view.x && h == view.y) { return; }
		ENGINE_LOG("onResize: ", w, " ", h);
		view = {w, h};

		// TODO: can we use a u16 for clip textures?
		colorTex.setStorage(TextureFormat::RGBA8, view);
		clipTex1.setStorage(TextureFormat::R32F, view);
		clipTex2.setStorage(TextureFormat::R32F, view);

		glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, colorTex.get(), 0);
		glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT1, clipTex1.get(), 0);
		glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT2, clipTex2.get(), 0);
	}

	void Context::onFocus(const bool has) {
		//ENGINE_LOG("onFocus: ", has);
		if (!has) {
			hoverValid = true;

			// Remove any hovers
			if (!hoverStack.empty()) {
				auto it = hoverStack.begin();
				auto end = hoverStack.end();
				auto next = it + 1;

				while (next != end) {
					(*it)->onEndChildHover(*next);
					it = next;
					++next;
				}

				(*it)->onEndHover();
				hoverStack.clear();
			}
		} else {
			hoverValid = false;
		}
	}

}
