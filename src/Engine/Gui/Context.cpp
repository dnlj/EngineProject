// Engine
#include <Engine/Camera.hpp>
#include <Engine/Gui/Context.hpp>
#include <Engine/Math/color.hpp>


namespace {
	template<bool Reverse, class Stack, class CanUseFunc, class CanUseChildFunc, class EndUseFunc, class EndUseChildFunc, class BeginUseFunc, class BeginUseChildFunc>
	void updateNestedBehaviour(
		Stack& front, Stack& back,
		CanUseFunc&& canUse, CanUseChildFunc&& canUseChild,
		EndUseFunc&& endUse, EndUseChildFunc&& endUseChild,
		BeginUseFunc&& beginUse, BeginUseChildFunc&& beginUseChild
	) {
		auto&& begin = [](Stack& stack) ENGINE_INLINE { if constexpr(Reverse) { return stack.rbegin(); } else { return stack.begin(); } };
		auto&& end = [](Stack& stack) ENGINE_INLINE { if constexpr(Reverse) { return stack.rend(); } else { return stack.end(); } };

		const auto aBegin = begin(back);
		const auto aEnd = end(back);
		auto aStop = aEnd;
		auto aCurr = aBegin;

		const auto bBegin = begin(front);
		const auto bEnd = end(front);
		auto bCurr = bBegin;
		
		// Find where stacks diverge
		while (aCurr != aEnd && bCurr != bEnd && *aCurr == *bCurr) {
			++aCurr;
			++bCurr;
		}

		// At this point aCurr and bCurr are the first element at which the stacks differ
		const auto aDiff = aCurr;

		{ // Validate the stack
			if (aCurr != aBegin) {
				--aCurr;
			}

			while (true) {
				if (aCurr == aStop) { break; }
				auto child = aCurr + 1;

				if (child == aStop) {
					if (!canUse(aCurr)) {
						aStop = aCurr;

						if (aCurr == aBegin) {
							// TODO: abort - stack empty
							ENGINE_WARN("TODO: abort focus");
						} else {
							--aCurr;
						}
					} else {
						// At this point aCurr has been validated and should be the last element
						ENGINE_DEBUG_ASSERT(child == aStop);
						break;
					}
				} else {
					if (!canUseChild(aCurr, child)) {
						aStop = aCurr + 1;
					} else {
						aCurr = child;
					}
				}
			}
		}

		if (bCurr == bEnd && bCurr != bBegin && aStop != aBegin) {
			if (*(bCurr-1) == *(aStop - 1)) {
				//ENGINE_INFO("Same target");
				return;
			}
		}

		// Call end events
		if (bBegin == bEnd) {
			//ENGINE_WARN("Empty b list");
		} else {
			auto bLast = bEnd - 1;
			endUse(bLast);

			if (bCurr != bBegin) {
				--bCurr;
			}

			while (true) {
				auto child = bLast;
				if (child == bCurr) { break; }
				--bLast;
				endUseChild(bLast, child);
			}
		}

		// Call begin events
		{
			if (aStop == aBegin) {
				//ENGINE_WARN("Empty a list");
			} else {
				aCurr = aStop < aDiff ? aStop : aDiff;

				if (aCurr != aBegin) {
					--aCurr;
				}

				while (true) {
					auto child = aCurr + 1;
					if (child == aStop) {
						beginUse(aCurr);
						break;
					}

					beginUseChild(aCurr, child);
					aCurr = child;
				}
			}

			if constexpr (Reverse) {
				back.erase(aEnd.base(), aStop.base());
			} else {
				back.erase(aStop, aEnd);
			}
		}

		// Cleanup
		front.swap(back);
	}

	class RootPanel final : public Engine::Gui::PanelT {
		public:
			using PanelT::PanelT;

			virtual void onBeginChildFocus(Panel* child) override {
				// Force the child to be on top
				addChild(child);
			};
			
			virtual bool onBeginActivate() override { return false; }

			// TODO: should we also have canHover/Focus return false?
	};
}


namespace Engine::Gui {
	Context::Context(ShaderManager& shaderManager, TextureManager& textureManager, Camera& camera) {
		quadShader = shaderManager.get("shaders/fullscreen_passthrough");
		polyShader = shaderManager.get("shaders/gui_poly");
		glyphShader = shaderManager.get("shaders/gui_glyph");
		configUserSettings();

		glProgramUniform1i(glyphShader->get(), 1, 0);
		glProgramUniform1i(polyShader->get(), 1, 0);

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
			GLuint attribLocation = -1;

			glCreateVertexArrays(1, &polyVAO);
			glCreateBuffers(1, &polyVBO);
			glVertexArrayVertexBuffer(polyVAO, bindingIndex, polyVBO, 0, sizeof(PolyVertex));

			glEnableVertexArrayAttrib(polyVAO, ++attribLocation);
			glVertexArrayAttribBinding(polyVAO, attribLocation, bindingIndex);
			glVertexArrayAttribFormat(polyVAO, attribLocation, 4, GL_FLOAT, GL_FALSE, offsetof(PolyVertex, color));

			glEnableVertexArrayAttrib(polyVAO, ++attribLocation);
			glVertexArrayAttribBinding(polyVAO, attribLocation, bindingIndex);
			glVertexArrayAttribFormat(polyVAO, attribLocation, 2, GL_FLOAT, GL_FALSE, offsetof(PolyVertex, texCoord));

			glEnableVertexArrayAttrib(polyVAO, ++attribLocation);
			glVertexArrayAttribBinding(polyVAO, attribLocation, bindingIndex);
			glVertexArrayAttribFormat(polyVAO, attribLocation, 2, GL_FLOAT, GL_FALSE, offsetof(PolyVertex, pos));
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
		}

		registerPanel(nullptr); // register before everything else so nullptr = id 0

		root = new RootPanel{this};
		root->setPos({0,0});
		registerPanel(root);

		///////////////////////////////////////////////////////////////////////////////

		// TODO: ideally these could just be "Arial" and "Consola" not actual paths.
		theme.fonts.header = fontManager.createFont("assets/arial.ttf", 32);
		theme.fonts.body = fontManager.createFont("assets/consola.ttf", 12);
		//font_a = fontManager.createFont("assets/EmojiOneColor.otf", 32);
		//font_a = fontManager.createFont("assets/FiraCode-Regular.ttf", 32);
		//font_b = fontManager.createFont("assets/arial.ttf", 128);

		const auto rgb = [&](glm::vec4 v) ENGINE_INLINE { return Math::cvtApproxRGBToLinear(v); };
		const auto hsl = [&](glm::vec4 v) ENGINE_INLINE { return rgb(Math::cvtHSLtoRGB(v)); };
		constexpr auto s = 0.70;
		constexpr auto l = 0.63;
		constexpr auto a = 0.33;

		theme.sizes = {
			.pad1 = 4.0f,
		};

		theme.colors = {
			.foreground = rgb({1, 0, 0, 1}),

			.background = rgb({0.1, 0.1, 0.2, a}),
			.background2 = rgb({0.1, 0.1, 0.4, a}),
			.backgroundAlt = rgb({1,0,0,1}),

			.title = rgb({0.1, 0.1, 0.1, 1}),

			.accent = hsl({202, s, l, 1}),
			.feature = {0.9, 0.9, 0.9, 1},

			.button = hsl({202, s, l, 1}),
		};

		{
			Image img{PixelFormat::RGB8, {1,1}};
			memset(img.data(), 0xFF, img.sizeBytes());
			defaultTexture.setStorage(TextureFormat::RGB8, img.size());
			defaultTexture.setImage(img);
		}

		activeTexture = defaultTexture;
		resetDraw();
	}

	Context::~Context() {
		glDeleteVertexArrays(1, &quadVAO);
		glDeleteBuffers(1, &quadVBO);

		glDeleteFramebuffers(1, &fbo);
		glDeleteVertexArrays(1, &polyVAO);
		glDeleteBuffers(1, &polyVBO);

		glDeleteVertexArrays(1, &glyphVAO);
		glDeleteBuffers(1, &glyphVBO);

		deletePanel(root);
	}

	void Context::configUserSettings() {
		#ifdef ENGINE_OS_WINDOWS
			cursorBlinkRate = std::chrono::milliseconds{GetCaretBlinkTime()};
			clickRate = std::chrono::milliseconds{GetDoubleClickTime()};
			clickSize = {GetSystemMetrics(SM_CXDOUBLECLK), GetSystemMetrics(SM_CYDOUBLECLK)};
			
			if (UINT out = 3; SystemParametersInfo(SPI_GETWHEELSCROLLCHARS, 0, &out, 0)) {
				scrollChars = static_cast<float32>(out);
			}

			if (UINT out = 3; SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &out, 0)) {
				scrollLines = static_cast<float32>(out);
			}

			ENGINE_LOG("GUI Blink Rate: ", Clock::Milliseconds{cursorBlinkRate}.count());
			ENGINE_LOG("GUI Click Rate: ", Clock::Milliseconds{clickRate}.count());
			ENGINE_LOG("GUI Click Size: ", clickSize);
			ENGINE_LOG("GUI Scroll Lines: ", scrollLines);
			ENGINE_LOG("GUI Scroll Chars: ", scrollChars);
		#else
			#error TODO: impl for non-Windows
			ENGINE_WARN("Not implemented for non-Windows");
		#endif
	}
	
	void Context::updateCursor() {
		#ifdef ENGINE_OS_WINDOWS
			::SetCursor(::LoadCursorW(0, MAKEINTRESOURCEW(currentCursor)));
		#else
			#error TODO: Implement cursors for non-Windows systems
		#endif
	}

	void Context::setIMEPosition(const glm::vec2 pos) {
		#if ENGINE_OS_WINDOWS
			// TODO: This only works the first time the system creats an ime window after each focus.
			// TODO: cont. to fix this i think we need to mes with WM_IME_* messages. See 04kVYW2Y for details.
			if (!nativeHandle) { ENGINE_WARN("Unable to set IME position."); return; }

			COMPOSITIONFORM comp = {};
			comp.dwStyle = CFS_POINT;
			comp.ptCurrentPos = {static_cast<LONG>(pos.x), static_cast<LONG>(pos.y)};

			CANDIDATEFORM cand = {};
			cand.dwStyle = CFS_EXCLUDE;
			cand.ptCurrentPos = comp.ptCurrentPos;
			//cand.rcArea;

			const auto handle = static_cast<HWND>(nativeHandle);
			const auto ctx = ::ImmGetContext(handle);
			// TODO: once 04kVYW2Y is fixed: ::ImmSetCandidateWindow(ctx, &cand);
			::ImmSetCompositionWindow(ctx, &comp);
			::ImmReleaseContext(handle, ctx);
		#else
			#error TODO: implement for non-Windows
		#endif
	}
	
	void Context::render() {
		if (!hoverValid) {
			updateHover();
			hoverValid = true;
		}

		while (currPanelUpdateFunc < panelUpdateFunc.size()) {
			auto& [panel, func] = panelUpdateFunc[currPanelUpdateFunc];
			if (panel->isEnabled()) { func(panel); }
			++currPanelUpdateFunc;
		}
		currPanelUpdateFunc = 0;

		if (auto focus = getFocus()) {
			for (auto act : focusActionQueue) {
				focus = getFocus();
				while (focus && !focus->onAction(act)) {
					focus = focus->getParent();
				}
			}
		}
		focusActionQueue.clear();

		if (auto hover = getHover()) {
			for (auto act : hoverActionQueue) {
				hover = getHover();
				while (hover && !hover->onAction(act)) {
					hover = hover->getParent();
				}
			}
		}
		hoverActionQueue.clear();

		// Update font buffers
		fontManager.updateAllFontDataBuffers();

		glEnable(GL_BLEND);
		glEnable(GL_SCISSOR_TEST);
		glBlendFuncSeparatei(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		glClearTexImage(colorTex.get(), 0, GL_RGB, GL_FLOAT, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		// DFS traversal
		for (Panel* curr = root; curr;) {
			drawOffset = curr->getPos();
			curr->render();

			if (auto* child = curr->getFirstChild()) {
				curr = child;
			} else if (auto* sib = curr->getNextSibling()) {
				curr = sib;
			} else {
				while (curr = curr->getParent()) {
					if (sib = curr->getNextSibling()) {
						curr = sib;
						break;
					}
				}
				if (!curr) { break; }
			}
		}

		// Needed to populate count of last draw groups
		if (auto last = polyDrawGroups.rbegin(); last != polyDrawGroups.rend()) {
			last->count = static_cast<int32>(polyVertexData.size()) - last->offset;
		}
		if (auto last = glyphDrawGroups.rbegin(); last != glyphDrawGroups.rend()) {
			last->count = static_cast<int32>(glyphVertexData.size()) - last->offset;
		}

		// Draw groups
		ENGINE_DEBUG_ASSERT(clipStack.size() == 1, "Mismatched push/pop clip");
		flushDrawBuffer();

		// Draw to main framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glBindVertexArray(quadVAO);
		glUseProgram(quadShader->get());
		glBindTextureUnit(0, colorTex.get());
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Reset buffers
		glDisable(GL_SCISSOR_TEST);
		glDisable(GL_BLEND);
	}

	void Context::flushDrawBuffer() {
		// Draw polys
		if (const auto count = polyVertexData.size()) {
			const auto size = count * sizeof(polyVertexData[0]);
			// Update polygon vertex buffer
			if (size > polyVBOCapacity) {
				polyVBOCapacity = static_cast<GLsizei>(polyVertexData.capacity() * sizeof(polyVertexData[0]));
				glNamedBufferData(polyVBO, polyVBOCapacity, nullptr, GL_DYNAMIC_DRAW);
			}
			glNamedBufferSubData(polyVBO, 0, size, polyVertexData.data());
		}
		
		// Update glyph vertex buffer
		if (const auto count = glyphVertexData.size()) {
			const GLsizei newSize = static_cast<GLsizei>(count * sizeof(GlyphVertex));
			if (newSize > glyphVBOCapacity) {
				glyphVBOCapacity = newSize;
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

		for (int32 zindex = 0; zindex <= renderState.zindex;) {
			if (currPolyGroup != lastPolyGroup && currPolyGroup->zindex == zindex) {
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
					glDrawArrays(GL_TRIANGLES, currPolyGroup->offset, currPolyGroup->count);
					++currPolyGroup;
					++zindex;
				} while (currPolyGroup != lastPolyGroup && currPolyGroup->zindex == zindex);
			}

			// Draw glyphs
			if (currGlyphGroup != lastGlyphGroup && currGlyphGroup->zindex == zindex) {
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
					++zindex;
				} while (currGlyphGroup != lastGlyphGroup && currGlyphGroup->zindex == zindex);
			}
		}
		resetDraw();
	}

	void Context::resetDraw() {
		clipStack.clear();
		clipStack.push_back({{0,0}, view});

		renderState.zindex = -1;

		polyVertexData.clear();
		polyDrawGroups.clear();

		glyphVertexData.clear();
		glyphDrawGroups.clear();
	}

	void Context::nextDrawGroupPoly() {
		if (polyDrawGroups.empty()) {
			++renderState.zindex;
			polyDrawGroups.emplace_back();
		}

		const auto sz = static_cast<int32>(polyVertexData.size());
		auto& prev = polyDrawGroups.back();
		prev.count += sz - prev.offset;

		const auto setup = [&](PolyDrawGroup& group) ENGINE_INLINE {
			group.zindex = renderState.zindex;
			group.offset = sz;
			group.clip = clipStack.back();
			group.tex = activeTexture;
		};
		
		// Figure out if we need a new group, can reuse an empty group, or extend the previous group
		if (prev.count == 0) {
			setup(prev);
		} else if (prev.zindex != renderState.zindex
			|| prev.clip != clipStack.back()
			|| prev.tex != activeTexture) {
			++renderState.zindex;
			setup(polyDrawGroups.emplace_back());
		} else {
			// No group change needed, extend the previous group
		}
	}
	
	void Context::nextDrawGroupGlyph() {
		if (glyphDrawGroups.empty()) {
			++renderState.zindex;
			glyphDrawGroups.emplace_back();
		}

		const auto sz = static_cast<int32>(glyphVertexData.size());
		auto& prev = glyphDrawGroups.back();
		prev.count = sz - prev.offset;

		const auto setup = [&](GlyphDrawGroup& group) ENGINE_INLINE {
			group.zindex = renderState.zindex;
			group.offset = sz;
			group.clip = clipStack.back();
			group.font = renderState.font;
		};

		// Figure out if we need a new group, can reuse an empty group, or extend the previous group
		if (prev.count == 0) {
			setup(prev);
		} else if (prev.zindex != renderState.zindex
			|| prev.font != renderState.font
			|| prev.clip != clipStack.back()) {
			++renderState.zindex;
			setup(glyphDrawGroups.emplace_back());
		} else {
			// No group change needed, extend the previous group
		}
	}

	void Context::pushClip(Bounds bounds) {
		const auto last = clipStack.back();
		bounds = bounds.intersect(last);
		clipStack.push_back(bounds);
		nextDrawGroupPoly();
	}
			
	void Context::popClip() {
		ENGINE_DEBUG_ASSERT(!clipStack.empty(), "Attempting to pop empty clipping stack");
		clipStack.pop_back();
		nextDrawGroupPoly();
	}

	void Context::drawTexture(TextureHandle2D tex, glm::vec2 pos, glm::vec2 size) {
		const auto old = activeTexture;
		activeTexture = tex;
		nextDrawGroupPoly();

		drawVertex(pos, {0,1});
		drawVertex(pos + glm::vec2{0, size.y}, {0,0});
		drawVertex(pos + size, {1,0});
		drawVertex(pos + size, {1,0});
		drawVertex(pos + glm::vec2{size.x, 0}, {1,1});
		drawVertex(pos, {0,1});

		activeTexture = old;
	}

	void Context::drawPoly(ArrayView<const glm::vec2> points, glm::vec4 color) {
		ENGINE_DEBUG_ASSERT(points.size() >= 3, "Must have at least three points");
		nextDrawGroupPoly();

		auto begin = points.begin();
		auto curr = begin + 1;
		auto next = curr + 1;
		auto end = points.end();

		while (next < end) {
			drawTri(*begin, *curr, *next, color);
			curr = next;
			++next;
		}
	}

	void Context::drawRect(glm::vec2 pos, glm::vec2 size, glm::vec4 color) {
		nextDrawGroupPoly();
		drawVertex(pos, color);
		drawVertex(pos + glm::vec2{0, size.y}, color);
		drawVertex(pos + size, color);
		drawVertex(pos + size, color);
		drawVertex(pos + glm::vec2{size.x, 0}, color);
		drawVertex(pos, color);
	}
	
	void Context::drawLine(glm::vec2 a, glm::vec2 b, float32 width, glm::vec4 color) {
		const auto t = glm::normalize(b - a);
		const auto n = width * 0.5f * glm::vec2{-t.y, t.x};
		drawPoly({a - n, a + n, b + n, b - n}, color);
	}

	void Context::drawString(glm::vec2 pos, const ShapedString* fstr) {
		ENGINE_DEBUG_ASSERT(fstr->getFont() != nullptr, "Attempting to draw string with null font.");
		
		const auto glyphShapeData = fstr->getGlyphShapeData();
		auto font = fstr->getFont();
		pos += drawOffset;

		renderState.font = font;
		nextDrawGroupGlyph();

		for (const auto& data : glyphShapeData) {
			const uint32 index = font->getGlyphIndex(data.index);
			glyphVertexData.push_back({
				.pos = glm::round(pos + data.offset),
				.index = index,
			});
			pos += data.advance;
		}
	}

	void Context::unsetActive() {
		active->onEndActivate();
		active = nullptr;
	}

	void Context::updateHover() {
		auto&& canUse = [](auto&& it) ENGINE_INLINE { return (*it)->canHover(); };
		auto&& canUseChild = [c=cursor](auto&& itP, auto&& itC) ENGINE_INLINE {
			return (*itP)->canHoverChild(*itC) && (*itC)->getBounds().contains(c);
		};

		auto&& endUse = [](auto&& it) ENGINE_INLINE { return (*it)->onEndHover(); };
		auto&& endUseChild = [](auto&& itP, auto&& itC) ENGINE_INLINE { return (*itP)->onEndChildHover(*itC); };

		auto&& beginUse = [](auto&& it) ENGINE_INLINE { return (*it)->onBeginHover(); };
		auto&& beginUseChild = [](auto&& itP, auto&& itC) ENGINE_INLINE { return (*itP)->onBeginChildHover(*itC); };

		hoverStackBack.clear();
		auto curr = root;

		// We traverse children in reverse order so that the results match what is rendered when children overlap
		// Manually check root since it doesn't have a parent (bounds checking would be skipped because of canUseChild)
		if (curr->getBounds().contains(cursor)) {
			hoverStackBack.push_back(curr);
			curr = curr->getLastChild();

			while (curr) {
				auto parent = curr->getParent();
				if (parent && canUseChild(&parent, &curr)) {
					hoverStackBack.push_back(curr);
					curr = curr->getLastChild();
				} else {
					curr = curr->getPrevSibling();
				}
			}
		}

		updateNestedBehaviour<false>(hoverStack, hoverStackBack, canUse, canUseChild, endUse, endUseChild, beginUse, beginUseChild);

		hover = hoverStack.empty() ? nullptr : hoverStack.back();
	}

	void Context::setFocus(Panel* panel) {
		focusStackBack.clear();
		for (auto curr = panel; curr != nullptr; curr = curr->getParent()) {
			focusStackBack.push_back(curr);
		}

		auto&& canUse = [](auto&& it) ENGINE_INLINE { return (*it)->canFocus(); };
		auto&& canUseChild = [](auto&& itP, auto&& itC) ENGINE_INLINE { return (*itP)->canFocusChild(*itC); };

		auto&& endUse = [](auto&& it) ENGINE_INLINE { return (*it)->onEndFocus(); };
		auto&& endUseChild = [](auto&& itP, auto&& itC) ENGINE_INLINE { return (*itP)->onEndChildFocus(*itC); };

		auto&& beginUse = [](auto&& it) ENGINE_INLINE { return (*it)->onBeginFocus(); };
		auto&& beginUseChild = [](auto&& itP, auto&& itC) ENGINE_INLINE { return (*itP)->onBeginChildFocus(*itC); };

		updateNestedBehaviour<true>(focusStack, focusStackBack, canUse, canUseChild, endUse, endUseChild, beginUse, beginUseChild);

		focus = focusStack.empty() ? nullptr : focusStack.front();
	}

	void Context::deletePanel(Panel* panel, bool isChild) {
		// Clear from active
		if (panel == active) { unsetActive(); }

		if (!isChild) {
			// Clear from focus
			for (auto curr = focus; curr; curr = curr->getParent()) {
				if (curr == panel) {
					setFocus(panel->getParent());
					break;
				}
			}

			// Clear from hover
			if (auto p = panel->getParent()) {
				p->removeChild(panel);
			}
			updateHover();
		}

		// Delete children
		auto child = panel->getFirstChildRaw();
		while (child) {
			const auto next = child->getNextSiblingRaw();
			deletePanel(child, true);
			child = next;
		}
				
		clearPanelUpdateFuncs(panel);
		deregisterMouseMove(panel);
		deregisterTextCallback(panel);
		deregisterPanel(panel);
		delete panel;
	}

	void Context::setClipboard(std::string_view view) {
		#if !ENGINE_OS_WINDOWS
			#error TODO: impl for non Windows
		#endif

		const auto handle = static_cast<HWND>(nativeHandle);
		if (!handle) {
			ENGINE_WARN("No native handle set");
			return;
		}

		if (!::OpenClipboard(handle)) {
			ENGINE_WARN("Unable to open clipboard");
			return;
		}

		::EmptyClipboard();

		std::wstring convBuffer;
		convBuffer.resize(view.size());
		static_assert(sizeof(convBuffer[0]) == 2, "Assumes a two byte wide char");

		// Convert to UTF-16
		::MultiByteToWideChar(CP_UTF8, 0,
			view.data(), static_cast<int>(view.size()),
			convBuffer.data(), static_cast<int>(convBuffer.size())
		);
		convBuffer.resize(std::wcslen(convBuffer.data()) + 1); // + 1 for null
		convBuffer.back() = 0;

		const auto sz = convBuffer.size() * 2;
		if (auto mem = ::GlobalAlloc(GMEM_MOVEABLE, sz)) {
			if (auto ptr = ::GlobalLock(mem)) {
				memcpy(ptr, convBuffer.data(), sz);
				::GlobalUnlock(mem);
			} else {
				ENGINE_WARN("Unable to lock win32 memory");
			}
			::SetClipboardData(CF_UNICODETEXT, mem);
		}
		::CloseClipboard();
	}

	std::string Context::getClipboardText() const {
		#if !ENGINE_OS_WINDOWS
			#error TODO: impl for non Windows
		#endif
		
		if (!::IsClipboardFormatAvailable(CF_UNICODETEXT)) {
			return {};
		}

		const auto handle = static_cast<HWND>(nativeHandle);
		if (!handle) {
			ENGINE_WARN("No native handle set");
			return {};
		}

		if (!::OpenClipboard(handle)) {
			ENGINE_WARN("Unable to open clipboard");
			return {};
		}

		std::wstring temp;
		if (const auto mem = ::GetClipboardData(CF_UNICODETEXT)) {
			if (const auto ptr = ::GlobalLock(mem)) {
				temp = reinterpret_cast<const WCHAR*>(ptr);
				::GlobalUnlock(mem);
			}
		}

		::CloseClipboard();

		if (temp.empty()) { return {}; }

		std::string result;
		const auto sz = ::WideCharToMultiByte(CP_UTF8, 0, temp.data(), static_cast<int>(temp.size()), nullptr, 0, nullptr, nullptr);
		if (sz == 0) { return {}; }

		result.resize(sz);
		::WideCharToMultiByte(CP_UTF8, 0, temp.data(), static_cast<int>(temp.size()), result.data(), static_cast<int>(result.size()), nullptr, nullptr);
		return result;
	}

	void Context::focusHover() {
		setFocus(getHover());
	}

	bool Context::onActivate(const bool state, Clock::TimePoint time) {
		if (state) {
			const auto isSequentialActivate = [&]() ENGINE_INLINE {
				if (time - clickLastTime > clickRate) { return false; }

				const auto diff = 2.0f * glm::abs(clickLastPos - getCursor());
				if (diff.x >= clickSize.x && diff.y >= clickSize.y) { return false; }

				return true;
			};

			if (isSequentialActivate()) {
				++activateCount;
			} else {
				activateCount = 1;
			}
			clickLastPos = getCursor();
			clickLastTime = time;

			auto target = getFocus();

			while (target) {
				if (target->onBeginActivate()) {
					active = target;
					return true;
				}
				target = target->getParent();
			}

			active = nullptr;
			activateCount = 0;
			return false;
		} else if (active) {
			unsetActive();
			return true;
		}
		return false;
	}

	bool Context::onMouse(const Engine::Input::InputEvent event) {
		//ENGINE_LOG("onMouse:",
		//	" ", event.state.value,
		//	" ", (int)event.state.id.code,
		//	" ", (int)event.state.id.type,
		//	" ", (int)event.state.id.device,
		//	" @ ", Engine::Clock::Seconds{event.time.time_since_epoch()}.count()
		//);
		return false;
	}

	bool Context::onMouseMove(const Engine::Input::InputEvent event) {
		//ENGINE_LOG("onMouseMove:", " ", event.state.id.code, " ", event.state.valuef, " @ ", Engine::Clock::Seconds{event.time.time_since_epoch()}.count());
		cursor = event.state.value.f32v2;
		hoverValid = false;

		for (auto& [_, cb] : mouseMoveCallbacks) {
			cb(cursor);
		}

		return false;
	}

	bool Context::onMouseWheel(const Engine::Input::InputEvent event) {
		// ENGINE_LOG("onMouseWheel: ", event.state.value, " @ ", Engine::Clock::Seconds{event.time.time_since_epoch()}.count());
		return false;
	}

	bool Context::onKey(const Engine::Input::InputEvent event) {
		// ENGINE_LOG("onKey: ", event.state.value, " @ ", Engine::Clock::Seconds{event.time.time_since_epoch()}.count());
		return false;
	}

	bool Context::onText(std::string_view str) {
		// Filter out non-printable ascii
		constexpr auto isPrintable = [](unsigned char c) -> bool {
			if (c  < 0x20 || c == 0x7F) {
				/*switch (c) {
					case '\n':
					case '\r':
					case '\t': return true;
				}*/
				return false;
			}
			return true;
		};

		if (str.size() == 1) {
			if (!isPrintable(str[0])) {
				return false;
			}
		} else {
			auto curr = &str[0];
			auto end = curr + str.size();

			textBuffer.clear();

			auto prev = curr;
			while (curr != end) {
				if (!isPrintable(*curr)) {
					textBuffer.append(prev, curr);
					prev = ++curr;
					continue;
				}
				++curr;
			}

			textBuffer.append(prev, curr);
			str = textBuffer;
		}

		for (auto& [panel, cb] : textCallbacks) {
			if (panel->isEnabled() && cb(str)) { return true; }
		}

		return false;
	}

	void Context::onResize(const int32 w, const int32 h) {
		if (w == view.x && h == view.y) { return; }

		view = {w, h};
		root->setSize(view);
		colorTex.setStorage(TextureFormat::RGBA8, view);
		glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, colorTex.get(), 0);

		const auto view2 = 2.0f / view;
		glProgramUniform2fv(polyShader->get(), 0, 1, &view2.x);
		glProgramUniform2fv(glyphShader->get(), 0, 1, &view2.x);
	}

	void Context::onFocus(const bool has) {
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
			updateCursor();
		}
	}

}
