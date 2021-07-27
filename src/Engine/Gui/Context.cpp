// FreeType
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>

// Engine
#include <Engine/Gui/Context.hpp>
#include <Engine/Gui/Button.hpp> // TODO: rm


namespace Engine::Gui {
	Context::Context(Engine::EngineInstance& engine) {
		quadShader = engine.shaderManager.get("shaders/fullscreen_passthrough");
		polyShader = engine.shaderManager.get("shaders/gui_poly");
		glyphShader = engine.shaderManager.get("shaders/gui_glyph");
		view = engine.camera.getScreenSize();

		glProgramUniform1i(glyphShader->get(), 2, 1);

		glCreateFramebuffers(1, &fbo);

		{
			const glm::vec2 quadData[] = {
				{1,1}, {-1,1}, {-1,-1},
				{-1,-1}, {1,-1}, {1,1},
			};

			glCreateBuffers(1, &quadVBO);
			glNamedBufferData(quadVBO, sizeof(quadData), &quadData, GL_STATIC_DRAW);

			glCreateVertexArrays(1, &quadVAO);
			glVertexArrayVertexBuffer(quadVAO, 0, quadVBO, 0, sizeof(quadData[0]));
			glEnableVertexArrayAttrib(quadVAO, 0);
			glVertexArrayAttribBinding(quadVAO, 0, 0);
			glVertexArrayAttribFormat(quadVAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
		}

		{
			constexpr static GLuint bindingIndex = 0;

			glCreateVertexArrays(1, &polyVAO);
			glCreateBuffers(1, &polyVBO);
			glVertexArrayVertexBuffer(polyVAO, bindingIndex, polyVBO, 0, sizeof(PolyVertex));

			glEnableVertexArrayAttrib(polyVAO, 0);
			glVertexArrayAttribBinding(polyVAO, 0, bindingIndex);
			glVertexArrayAttribFormat(polyVAO, 0, 4, GL_FLOAT, GL_FALSE, offsetof(PolyVertex, color));

			glEnableVertexArrayAttrib(polyVAO, 1);
			glVertexArrayAttribBinding(polyVAO, 1, bindingIndex);
			glVertexArrayAttribFormat(polyVAO, 1, 2, GL_FLOAT, GL_FALSE, offsetof(PolyVertex, pos));

			glEnableVertexArrayAttrib(polyVAO, 2);
			glVertexArrayAttribBinding(polyVAO, 2, bindingIndex);
			glVertexArrayAttribFormat(polyVAO, 2, 1, GL_FLOAT, GL_FALSE, offsetof(PolyVertex, id));
			//glVertexArrayAttribIFormat(polyVAO, 2, 1, GL_UNSIGNED_INT, offsetof(PolyVertex, id));
		
			glEnableVertexArrayAttrib(polyVAO, 3);
			glVertexArrayAttribBinding(polyVAO, 3, bindingIndex);
			glVertexArrayAttribFormat(polyVAO, 3, 1, GL_FLOAT, GL_FALSE, offsetof(PolyVertex, pid));
			//glVertexArrayAttribIFormat(polyVAO, 3, 1, GL_UNSIGNED_INT, offsetof(PolyVertex, pid));
		}

		{
			constexpr static GLuint bindingIndex = 0;

			glCreateBuffers(1, &glyphVBO);

			glCreateVertexArrays(1, &glyphVAO);
			glVertexArrayVertexBuffer(glyphVAO, bindingIndex, glyphVBO, 0, sizeof(GlyphVertex));

			glEnableVertexArrayAttrib(glyphVAO, bindingIndex);
			glVertexArrayAttribBinding(glyphVAO, 0, bindingIndex);
			glVertexArrayAttribFormat(glyphVAO, 0, 2, GL_FLOAT, GL_FALSE, offsetof(GlyphVertex, pos));

			glEnableVertexArrayAttrib(glyphVAO, 1);
			glVertexArrayAttribBinding(glyphVAO, 1, bindingIndex);
			glVertexArrayAttribIFormat(glyphVAO, 1, 1, GL_UNSIGNED_INT, offsetof(GlyphVertex, index));

			glEnableVertexArrayAttrib(glyphVAO, 2);
			glVertexArrayAttribBinding(glyphVAO, 2, bindingIndex);
			glVertexArrayAttribFormat(glyphVAO, 2, 1, GL_FLOAT, GL_FALSE, offsetof(GlyphVertex, parent));
		}

		registerPanel(nullptr); // register before everything else so nullptr = id 0

		root = new Panel{};
		root->setPos({25, 50});
		root->setSize({512, 256});
		registerPanel(root);

		///////////////////////////////////////////////////////////////////////////////

		font_a = fontManager.createFont("assets/arial.ttf", 32);
		font_b = fontManager.createFont("assets/consola.ttf", 12);
		//font_b = fontManager.createFont("assets/arial.ttf", 128);

		{
			auto child = new Button{};
			root->addChild(child);
			child->setPos({0, 0});
			child->setSize({64, 300});
			registerPanel(child);

			child->label = R"(Hello, world!)";
			child->label.setFont(font_b);
			fontManager.shapeString(child->label);

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

		{
			auto child = new Button{};
			root->addChild(child);
			child->setPos({256, 10});
			child->setSize({128, 64});

			child->label = R"(This is a test button)";
			child->label.setFont(font_a);
			fontManager.shapeString(child->label);

			registerPanel(child);
		}
	}

	Context::~Context() {
		glDeleteVertexArrays(1, &quadVAO);
		glDeleteBuffers(1, &quadVBO);

		glDeleteFramebuffers(1, &fbo);
		glDeleteVertexArrays(1, &polyVAO);
		glDeleteBuffers(1, &polyVBO);

		glDeleteVertexArrays(1, &glyphVAO);
		glDeleteBuffers(1, &glyphVBO);

		delete root;
	}
	
	void Context::render() {
		if (!hoverValid) {
			updateHover();
			hoverValid = true;
		}

		const Panel* curr = root;
		renderState.layer = 0;
		renderState.offset = {};
		polyDrawGroups.emplace_back().offset = 0;

		// TODO: probably move to own function
		// Breadth first traversal
		while (true) {
			// Traverse siblings
			while (curr) {
				if (curr->firstChild) {
					bfsNext.emplace_back(
						renderState.offset + curr->getPos(),
						curr->firstChild
					);
				}

				if (false) {}
				else if (curr == getActive()) { renderState.color = glm::vec4{1, 0, 1, 0.2}; }
				else if (curr == getHover()) { renderState.color = glm::vec4{1, 1, 0, 0.2}; }
				else { renderState.color = glm::vec4{1, 0, 0, 0.2}; }

				renderState.current = curr;
				renderState.id = getPanelId(renderState.current);
				renderState.pid = getPanelId(renderState.current->parent);

				const auto oldOffset = renderState.offset;
				renderState.offset += curr->getPos();
				curr->render(*this);
				renderState.offset = oldOffset;
				curr = curr->nextSibling;
			}

			// Move to next layer if needed
			if (bfsCurr.empty()) {
				const auto vsz = static_cast<GLint>(polyVertexData.size());
				polyDrawGroups.back().count = vsz - polyDrawGroups.back().offset;

				bfsCurr.swap(bfsNext);
				if (bfsCurr.empty()) { break; }

				polyDrawGroups.emplace_back().offset = vsz;
				++renderState.layer;
			}

			// Next of current layer
			const auto& back = bfsCurr.back();
			curr = back.panel;
			renderState.offset = back.offset;
			bfsCurr.pop_back();
		}

		// Build glyph vertex buffer
		if (!stringsToRender.empty()){
			std::sort(stringsToRender.begin(), stringsToRender.end(), [](const StringData& a, const StringData& b){
				return (a.layer < b.layer)
					|| (a.layer == b.layer && a.str->getFont() < b.str->getFont());
			});

			// Build glyph draw groups
			int32 currLayer = stringsToRender.front().layer;
			Font currFont = stringsToRender.front().str->getFont();
			
			GlyphDrawGroup* group = &glyphDrawGroups.emplace_back();
			group->glyphSet = currFont;
			group->layer = currLayer;
			auto ascent = group->glyphSet->getAscent();

			for (const auto& strdat : stringsToRender) {
				if (strdat.layer != currLayer || currFont != strdat.str->getFont()) {
					currLayer = strdat.layer;
					currFont = strdat.str->getFont();

					GlyphDrawGroup next {
						.layer = currLayer,
						.offset = group->offset + group->count,
						.count = 0,
						.glyphSet = currFont,
					};
					group = &glyphDrawGroups.emplace_back(next);
					ascent = group->glyphSet->getAscent();
				}

				auto pos = strdat.pos;
				pos.y += ascent;
				renderString(*strdat.str, strdat.parent, pos, group->glyphSet);
				group->count += static_cast<int32>(strdat.str->getGlyphShapeData().size());
			}

			stringsToRender.clear();
		}

		{ // Update polygon vertex buffer
			// TODO: idealy we would only update if the data has actually changed
			const auto size = polyVertexData.size() * sizeof(polyVertexData[0]);
			if (size > polyVBOCapacity) {
				polyVBOCapacity = static_cast<GLsizei>(polyVertexData.capacity() * sizeof(PolyVertex));
				glNamedBufferData(polyVBO, polyVBOCapacity, nullptr, GL_DYNAMIC_DRAW);
			}
			glNamedBufferSubData(polyVBO, 0, size, polyVertexData.data());
			polyVertexData.clear();
		}
		
		{ // Update glyph vertex buffer
			const GLsizei newSize = static_cast<GLsizei>(glyphVertexData.size() * sizeof(GlyphVertex));
			if (newSize > glyphVBOCapacity) {
				ENGINE_INFO("glyphVBO(", glyphVBO, ") resize: ", newSize);
				glyphVBOCapacity = newSize;
				glNamedBufferData(glyphVBO, glyphVBOCapacity, nullptr, GL_DYNAMIC_DRAW);
			}
			glNamedBufferSubData(glyphVBO, 0, newSize, glyphVertexData.data());
			glyphVertexData.clear();
		}

		// Update font buffers
		fontManager.updateAllFontDataBuffers();

		glEnable(GL_BLEND);
		glBlendFuncSeparatei(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glBlendFunci(1, GL_ONE, GL_ZERO);

		glClearTexImage(colorTex.get(), 0, GL_RGB, GL_FLOAT, 0);
		glClearTexImage(clipTex1.get(), 0, GL_RGB, GL_FLOAT, 0);
		glClearTexImage(clipTex2.get(), 0, GL_RGB, GL_FLOAT, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		// TODO: create constexpr constants for managing texture units, currently 0 is clip and 1 is glyphs

		// We cant use glMultiDrawArrays because you can not read/write
		// the same texture. It may be possible to work around this
		// with glTextureBarrier but that isnt as widely supported.

		auto currGlyphDrawGroup = glyphDrawGroups.data();
		const auto lastGlyphDrawGroup = currGlyphDrawGroup + glyphDrawGroups.size();
		GLuint activeStage = 0;

		auto swapClipBuffers = [&]() ENGINE_INLINE {
			activeClipTex = !activeClipTex;
			const GLenum buffs[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 + activeClipTex};
			glNamedFramebufferDrawBuffers(fbo, 2, &buffs[0]);
			glBindTextureUnit(0, activeClipTex ? clipTex1.get() : clipTex2.get());
		};

		// Setup buffers and uniforms
		swapClipBuffers();
		glProgramUniform2fv(polyShader->get(), 0, 1, &view.x);
		glProgramUniform2fv(glyphShader->get(), 0, 1, &view.x);

		for (int32 layer = 0; layer < polyDrawGroups.size(); ++layer) {
			const auto first = polyDrawGroups[layer].offset;
			const auto count = polyDrawGroups[layer].count;

			// Draw polys
			{
				if (activeStage != polyVAO) {
					activeStage = polyVAO;
					glBindVertexArray(polyVAO);
					glUseProgram(polyShader->get());
				}

				glDrawArrays(GL_TRIANGLES, first, count);
			}

			swapClipBuffers();

			// Draw glyphs
			{
				if (currGlyphDrawGroup == lastGlyphDrawGroup) { continue; }
				if (currGlyphDrawGroup->layer != layer) { continue; }

				// Disable drawing to clip buffer
				glBlendFunci(1, GL_ZERO, GL_ONE);

				if (activeStage != glyphVAO) {
					activeStage = glyphVAO;
					glBindVertexArray(glyphVAO);
					glUseProgram(glyphShader->get());
				}

				FontGlyphSet* activeSet = nullptr;

				while (true) {
					if (currGlyphDrawGroup->glyphSet != activeSet) {
						activeSet = currGlyphDrawGroup->glyphSet;
						glBindTextureUnit(1, activeSet->getGlyphTexture().get());
						glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, activeSet->getGlyphDataBuffer());
					}

					//ENGINE_LOG("Draw Glyphs: layer(", currGlyphDrawGroup->layer, "), offset(", currGlyphDrawGroup->offset, ") count(", currGlyphDrawGroup->count, ")");
					glDrawArrays(GL_POINTS, currGlyphDrawGroup->offset, currGlyphDrawGroup->count);

					++currGlyphDrawGroup;
					if (currGlyphDrawGroup == lastGlyphDrawGroup) { break; }
					if (currGlyphDrawGroup->layer != layer) { break; }
				}

				// Enable drawing to clip buffer
				glBlendFunci(1, GL_ONE, GL_ZERO);
			}
		}

		// Draw to main framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glBindVertexArray(quadVAO);
		glUseProgram(quadShader->get());
		glBindTextureUnit(0, colorTex.get());
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Reset buffers
		glDisable(GL_BLEND);
		polyDrawGroups.clear();
		glyphDrawGroups.clear();
	}

	void Context::drawRect(const glm::vec2 pos, const glm::vec2 size) {
		const PanelId id = renderState.id;
		const PanelId pid = renderState.pid;
		const auto color = renderState.color;
		const auto offset = renderState.offset;

		polyVertexData.push_back({.color = color, .pos = offset + pos, .id = id, .pid = pid});
		polyVertexData.push_back({.color = color, .pos = offset + pos + glm::vec2{0, size.y}, .id = id, .pid = pid});
		polyVertexData.push_back({.color = color, .pos = offset + pos + size, .id = id, .pid = pid});

		polyVertexData.push_back({.color = color, .pos = offset + pos + size, .id = id, .pid = pid});
		polyVertexData.push_back({.color = color, .pos = offset + pos + glm::vec2{size.x, 0}, .id = id, .pid = pid});
		polyVertexData.push_back({.color = color, .pos = offset + pos, .id = id, .pid = pid});
	}

	void Context::drawString(glm::vec2 pos, const ShapedString* fstr) {
		stringsToRender.emplace_back(renderState.layer, renderState.id, pos + renderState.offset, fstr);
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
					// Make sure `it` is still `curr`
					// Only time this is the case is if the root panel fails the above test (`it` didnt get incremented)
					if (curr != nullptr) { --it; }
					break;
				}
			}

			// Call onEndChildHover on the appropriate nodes
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

	void Context::renderString(const ShapedString& str, PanelId parent, glm::vec2 base, FontGlyphSet* font) {
		const auto glyphShapeData = str.getGlyphShapeData();

		for (const auto& data : glyphShapeData) {
			const uint32 index = font->getGlyphIndex(data.index);
			glyphVertexData.push_back({
				.pos = glm::round(base + data.offset),
				.index = index,
				.parent = parent,
			});
			base += data.advance;
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
