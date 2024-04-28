// Engine
#include <Engine/Camera.hpp>
#include <Engine/Gfx/ShaderManager.hpp>
#include <Engine/Math/color.hpp>
#include <Engine/UI/Context.hpp>
#include <Engine/Win32/win32.hpp>


/**
 * Sanity check for working with panels.
 * 99% of the time we expect to be working with non-null and non-deleted panels.
 */
#define USABLE(Panel)\
	ENGINE_DEBUG_ASSERT((Panel) != nullptr);\
	ENGINE_DEBUG_ASSERT(!(Panel)->isDeleted());

namespace {
	/**
	 * Call the associated end and begin functions on a sequence of panels.
	 *
	 * The stacks are intended to be setup in order or reverse order from parent
	 * to child (depending on @p Reverse). It isn't strictly required that the
	 * panels have a parent/child relationship since we don't use any sibling,
	 * parent, or child functions here, but it would make the begin/end function
	 * parameters not make 100% sense.
	 * 
	 * This used to happen if you deleted a panel right before updating focus,
	 * but I think that should now be resolved within deferredDeletePanels. It
	 * might still be possible when you change a panels parent?
	 *
	 * When a panel is added to the active stack the begin functions are called
	 * on its parent and then it. When a panel is removed from the active stack
	 * the end functions are called on it and then its parent panel. The end
	 * functions are always called before the begin and in reverse order.
	 * Similar to constructors and destructors.
	 * 
	 * Example:
	 * ------------------------------------------------------------------------------
	 *   Going from stack one to stack two:
	 *     1. A > B > C > D
	 *     2. A > B > E > F
	 *   
	 *   Calls:
	 *     - end(D), endChild(C, D), end(C), endChild(B, C)
	 *     - beginChild(B, E), begin(E), beginChild(E, F), begin(F)
	 * ------------------------------------------------------------------------------
	 *
	 * @tparam Reverse Is the order of panel in each stack parent then child or child then parent.
	 * @param front The current/active/existing list of panels from the previous run.
	 * @param back The new set of panels to target.
	 */
	template<bool Reverse, class EndUseFunc, class EndUseChildFunc, class BeginUseFunc, class BeginUseChildFunc>
	void updateNestedBehaviour(
		std::vector<Engine::UI::Panel*>& front, std::vector<Engine::UI::Panel*>& back,
		EndUseFunc&& endUse, EndUseChildFunc&& endUseChild,
		BeginUseFunc&& beginUse, BeginUseChildFunc&& beginUseChild
	) {
		constexpr auto begin = [](auto& stack) ENGINE_INLINE { if constexpr(Reverse) { return stack.rbegin(); } else { return stack.begin(); } };
		constexpr auto end = [](auto& stack) ENGINE_INLINE { if constexpr(Reverse) { return stack.rend(); } else { return stack.end(); } };

		// A = new stack (back stack)
		const auto aBegin = begin(back);
		const auto aEnd = end(back);
		const auto aStop = aEnd;
		auto aCurr = aBegin;

		// B = old/active stack (front stack)
		const auto bBegin = begin(front);
		const auto bEnd = end(front);
		auto bCurr = bBegin;

		// Find where stacks diverge
		while (aCurr != aEnd && bCurr != bEnd && *aCurr == *bCurr) {
			++aCurr;
			++bCurr;
		}

		// At this point aCurr and bCurr are the first element at which the stacks differ
		ENGINE_DEBUG_ONLY(const auto aDiff = aCurr);

		// No change in the target.
		// - Neither stack is empty.
		// - The new stack is equal to or a subset of the old stack.
		// - The last item in each stack is equal.
		//
		// We need the additional check that `bCurr == aEnd-1` because it
		// could be that the back stack is a subset of the front stack. In that
		// case we still need to call some end functions and use the last item.
		if (bCurr == bEnd && bEnd != bBegin && aEnd != aBegin) {
			if (*(bCurr-1) == *(aEnd-1)) {
				//ENGINE_INFO("Same target");
				back.clear();
				return;
			}
		}

		// Call end events
		if (bBegin == bEnd) {
			// There new stack is empty. There is no target.
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
			if (aEnd == aBegin) {
				// The old stack is empty. There was no target.
				//ENGINE_WARN("Empty a list");
			} else {
				ENGINE_DEBUG_ASSERT(aCurr == aDiff);
				if (aCurr != aBegin) {
					--aCurr;
				}

				while (true) {
					 // TODO: Why is this true at this point? Particularly for the first iteration. Document.
					ENGINE_DEBUG_ASSERT(aCurr != aEnd);

					auto child = aCurr + 1;
					if (child == aEnd) {
						USABLE(*aCurr);
						beginUse(aCurr);
						break;
					}
					
					USABLE(*aCurr);
					USABLE(*child);
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
		back.clear();
	}

	class RootPanel final : public Engine::UI::PanelT {
		public:
			using PanelT::PanelT;

			virtual void onBeginChildFocus(Panel* child) override {
				USABLE(child);
				child->toTop();
			};
			
			virtual bool onBeginActivate() override { return false; }

			// TODO: should we also have canHover/Focus return false?
	};
}

namespace Engine::UI {
	Context::Context(Gfx::ShaderLoader& shaderLoader, Gfx::TextureLoader& textureLoader, Camera& camera)
		: DrawBuilder(shaderLoader, textureLoader) {

		quadShader = shaderLoader.get("shaders/fullscreen_passthrough");
		configUserSettings();

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

		registerPanel(nullptr); // register before everything else so nullptr = id 0

		root = new RootPanel{this};
		root->setPos({0,0});
		registerPanel(root);

		///////////////////////////////////////////////////////////////////////////////

		// TODO: ideally these could just be "Arial" and "Consola" not actual paths.
		theme.fonts.header = createFont("assets/arial.ttf", 32);
		theme.fonts.body = createFont("assets/consola.ttf", 12);
		//font_a = fontManager.createFont("assets/EmojiOneColor.otf", 32);
		//font_a = fontManager.createFont("assets/FiraCode-Regular.ttf", 32);
		//font_b = fontManager.createFont("assets/arial.ttf", 128);

		constexpr auto rgb = [](glm::vec4 v) ENGINE_INLINE { return Math::cvtApproxRGBToLinear(v); };
		constexpr auto hsl = [](glm::vec4 v) ENGINE_INLINE { return rgb(Math::cvtHSLtoRGB(v)); };
		constexpr auto s = 0.70;
		constexpr auto l = 0.63;
		constexpr auto a = 0.33;

		theme.sizes = {
			.pad1 = 4.0f,
		};

		theme.colors = {
			.foreground = rgb({1, 1, 1, 1}),
			.foregroundAlt = rgb({0, 0, 0, 1}),

			.background = rgb({0.1, 0.1, 0.2, a}),
			.background2 = rgb({0.1, 0.1, 0.4, a}),
			.backgroundAlt = rgb({1,0,0,1}),
			.backgroundAlt2 = rgb({0,1,0,1}),

			.title = rgb({0.1, 0.1, 0.1, 1}),

			.accent = hsl({202, s, l, 1}),
			.feature = {0.9, 0.9, 0.9, 1},

			.button = hsl({202, s, l, 1}),
		};
	}

	Context::~Context() {
		glDeleteVertexArrays(1, &quadVAO);
		glDeleteBuffers(1, &quadVBO);

		glDeleteFramebuffers(1, &fbo);

		deleteSiblings(root, root);
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

			// Win32 doesn't provide a autoscroll rate but deriving from click rate
			// is a good approximation based on the articles from Raymond Chen
			// See:
			//   Raymond Chen. "Autoscrolling on drag, part 1: Basic implementation", https://devblogs.microsoft.com/oldnewthing/20210125-00/?p=104757
			//   Raymond Chen. "Autoscrolling on drag, part 2: Why does scrolling go faster if I wiggle the mouse?", https://devblogs.microsoft.com/oldnewthing/20210126-00/?p=104759
			//   Raymond Chen. "Autoscrolling on drag, part 3: Dynamic autoscroll based on mouse position", https://devblogs.microsoft.com/oldnewthing/20210127-00/?p=104764
			//   Raymond Chen. "Autoscrolling on drag, part 4: Dynamic autoscroll based on escape velocity", https://devblogs.microsoft.com/oldnewthing/20210128-00/?p=104768
			//   Raymond Chen. "The double-click time tells the window manager how good your reflexes are", https://devblogs.microsoft.com/oldnewthing/20080423-00/?p=22623
			autoscrollRate = clickRate / 5;
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
		deleteDeferredPanels();

		if (!hoverValid) {
			updateHover();
		}

		while (currPanelUpdateFunc < panelUpdateFunc.size()) {
			auto& [id, panel, func] = panelUpdateFunc[currPanelUpdateFunc];
			if (panel) {
				if (panel->isEnabled()) { func(panel); }
			} else {
				func(panel);
			}
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

		// DFS traversal
		reset();
		for (Panel* curr = root; curr;) {
			ENGINE_DEBUG_ASSERT(!curr->isDeleted());
			ENGINE_DEBUG_ASSERT(curr->isEnabled());
			setOffset(curr->getPos());

			pushClip();
			setClip(curr->getBounds());

			curr->render();
			resetStyle();

			if (auto* child = curr->getFirstChild()) {
				curr = child;
			} else {
				while (true) {
					popClip();
					if (auto* sib = curr->getNextSibling()) {
						curr = sib;
						break;
					}
					if (!(curr = curr->getParent())) { break; }
				}
			}
		}
		//ENGINE_LOG2("--- End");
		
		glEnable(GL_BLEND);
		glEnable(GL_SCISSOR_TEST);
		glBlendFuncSeparatei(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glClearTexImage(colorTex.get(), 0, GL_RGB, GL_FLOAT, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		// Draw groups
		finish();
		draw();

		// Draw to main framebuffer
		glDisable(GL_SCISSOR_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glBindVertexArray(quadVAO);
		glUseProgram(quadShader->get());
		glBindTextureUnit(0, colorTex.get());
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Cleanup
		glDisable(GL_BLEND);
	}

	void Context::unsetActive() {
		// We must set active to null before we call onEndActivate in case
		// onEndActivate calls another function that might call unsetActive and
		// cause an infinite onEndActivate loop.
		//
		// For example deferredDeletePanel checks if the panel is active before
		// deleting it and calls unsetActive if it is. This might happen on a
		// window close button for example. The delete is trigged in
		// onEndActivate which in turn deletes the parent window.
		auto* wasActive = active;
		active = nullptr;
		wasActive->onEndActivate();
	}

	void Context::updateHover() {
		ENGINE_DEBUG_ASSERT(hoverGuard == false, "updateHover called recursively from one of the panel hover callbacks.");
		ENGINE_DEBUG_ONLY(hoverGuard = true);

		auto&& canUse = [](auto&& it) ENGINE_INLINE { return (*it)->canHover(); };
		auto&& canUseChild = [c=cursor](auto&& itP, auto&& itC) ENGINE_INLINE {
			return (*itP)->canHoverChild(*itC) && (*itC)->getBounds().contains(c);
		};

		auto&& endUse = [](auto&& it) ENGINE_INLINE { return (*it)->onEndHover(); };
		auto&& endUseChild = [](auto&& itP, auto&& itC) ENGINE_INLINE { return (*itP)->onEndChildHover(*itC); };

		auto&& beginUse = [](auto&& it) ENGINE_INLINE { return (*it)->onBeginHover(); };
		auto&& beginUseChild = [](auto&& itP, auto&& itC) ENGINE_INLINE { return (*itP)->onBeginChildHover(*itC); };

		ENGINE_DEBUG_ASSERT(hoverStackBack.empty());

		// We traverse children in reverse order (prev, not next sibling) so
		// that the results match what is rendered when children overlap. That
		// is, the last children are drawn on top of the first children so check
		// the last children first.
		for (auto curr = root; curr;) {
			const auto parent = curr->getParent();
			if (!parent || canUseChild(&parent, &curr)) {
				hoverStackBack.push_back(curr);
				curr = curr->getLastChild();
			} else {
				curr = curr->getPrevSibling();
			}

			// No previous sibling, step back to parent.
			while (curr == nullptr) {
				// Nothing is hoverable.
				if (hoverStackBack.empty()) {
					// Also break the outer loop since curr == nullptr
					break;
				}

				curr = hoverStackBack.back();

				// TODO: do we want this case to go to the parent? Or just no hover?
				// The panel has no hoverable children.
				if (canUse(&curr)) {
					// Also break the outer loop since curr == nullptr
					curr = nullptr;
					break;
				}

				curr = curr->getPrevSibling();
				hoverStackBack.pop_back();
			}
		}

		updateNestedBehaviour<false>(hoverStack, hoverStackBack, endUse, endUseChild, beginUse, beginUseChild);

		hover = hoverStack.empty() ? nullptr : hoverStack.back();
		hoverValid = true;
		ENGINE_DEBUG_ONLY(hoverGuard = false);
	}

	void Context::setFocus(Panel* panel) {
		USABLE(panel);
		ENGINE_DEBUG_ASSERT(focusGuard == false, "setFocus called recursively from one of the panel hover callbacks.");
		ENGINE_DEBUG_ONLY(focusGuard = true);

		auto&& canUse = [](auto&& it) ENGINE_INLINE { return (*it)->canFocus(); };
		auto&& canUseChild = [](auto&& itP, auto&& itC) ENGINE_INLINE { return (*itP)->canFocusChild(*itC); };

		auto&& endUse = [](auto&& it) ENGINE_INLINE { return (*it)->onEndFocus(); };
		auto&& endUseChild = [](auto&& itP, auto&& itC) ENGINE_INLINE { return (*itP)->onEndChildFocus(*itC); };

		auto&& beginUse = [](auto&& it) ENGINE_INLINE { return (*it)->onBeginFocus(); };
		auto&& beginUseChild = [](auto&& itP, auto&& itC) ENGINE_INLINE { return (*itP)->onBeginChildFocus(*itC); };

		ENGINE_DEBUG_ASSERT(focusStackBack.empty());

		{
			if (!canUse(&panel)) {
				// Can't focus the panel we attempted to focus. Since the focus
				// stack (back) is already empty (see above assert) we do nothing.
			} else {
				auto curr = panel;
				while (curr) {
					USABLE(curr);

					// This logic is a bit strange since the focus stack is build in
					// reverse order. If we cant use this panel we need to clear the
					// stack or else we will have "inaccesible" entries (missing
					// ancestors) with the wrong parents.
					const auto parent = curr->getParent();
					if (!parent || canUseChild(&parent, &curr)) {
						focusStackBack.push_back(curr);
					} else {
						focusStackBack.clear();
					}

					curr = parent;
				}
			}
		}

		updateNestedBehaviour<true>(focusStack, focusStackBack, endUse, endUseChild, beginUse, beginUseChild);

		focus = focusStack.empty() ? nullptr : focusStack.front();
		ENGINE_DEBUG_ONLY(focusGuard = false);
	}
	
	void Context::deferredDeletePanels(Panel* first, Panel* last) {
		ENGINE_DEBUG_ASSERT(first && last);
		const auto parent = first->getParent();
		if (parent && parent->isDeleted()) { return; }

		// Figure out if we might be in the focus stack
		Panel* nextFocus = nullptr;
		for (auto curr = focusStack.rbegin(), end = focusStack.rend(); curr != end; ++curr) {
			if (*curr == parent) {
				auto next = curr + 1;
				if (next != end) {
					// We have to store this off and check it in the next loop since
					// nextFocus might be a child of parent but not in [first, last]
					nextFocus = *next;
				}
				break;
			}
		}

		// Clean as much as we can without true deletion
		for (auto* curr = first;; curr = curr->getNextSiblingRaw()) {
			ENGINE_DEBUG_ASSERT(curr->getParent() == parent);
			ENGINE_DEBUG_ASSERT(parent != nullptr);

			// TODO: These aren't quite right. It is fine to delete things while
			//       ending focus, just not while beginning focus. With how things
			//       are currently implemented there can be things in the back stack
			//       that are filter by the canUse functions. Those panels should be
			//       fine to delete since they wont be in the new focus.
			ENGINE_DEBUG_ASSERT(!focusGuard || !std::ranges::contains(focusStackBack, curr), "Attempting to delete a panel while it is begin added to the focus stack.");
			ENGINE_DEBUG_ASSERT(!hoverGuard || !std::ranges::contains(hoverStackBack, curr), "Attempting to delete a panel while it is begin added to the hover stack.");

			// Clear from focus.
			// This case will happen if a child widget deletes its (grand)parent. A window
			// close button for example.
			if (curr == nextFocus) {
				setFocus(parent);
			}

			// Cleanup panel and children
			Panel::unsafe_setParent(curr, nullptr);
			cleanup(curr);

			// Queue for deletion.
			// Children are handled by deleteSiblings
			deleteQueue.push_back(curr);
			if (curr == last) { break; }
		}

		// Clear from active
		if (active && active->isDeleted()) { unsetActive(); }

		// Update hover
		for (auto panel : hoverStack) {
			if (panel == parent) {
				updateHover();
				break;
			}
		}

		// Remove from and update parent
		if (parent) {
			Panel::unsafe_orphanChildren(parent, first, last);
			parent->performLayout(); // TODO: Could we also defer this until after all panels have been REALLY deleted? For the case where you delete multiple children?
		}
	}

	void Context::cleanup(Panel* panel) {
		if (panel->isDeleted()) { return; }

		Panel::unsafe_markDeleted(panel);
		clearAllCallbacks(panel);
		for (auto child = panel->getFirstChildRaw(); child; child = child->getNextSiblingRaw()) {
			cleanup(child);
		}
	};

	void Context::deleteDeferredPanels() {
		for (auto p : deleteQueue) {
			deleteSiblings(p, p);
		}
		deleteQueue.clear();
	}

	void Context::deleteSiblings(Panel* first, Panel* last) {
		for (auto* curr = first;;) {
			if (curr->getFirstChildRaw()) {
				deleteSiblings(curr->getFirstChildRaw(), curr->getLastChildRaw());
			}

			const auto next = curr->getNextSiblingRaw();
			delete curr;

			if (curr == last) { break; }
			curr = next;
		}
	}

	void Context::setClipboard(ArrayView<const std::string_view> array) const {
		#if !ENGINE_OS_WINDOWS
			#error TODO: impl for non Windows
		#endif

		const auto handle = static_cast<HWND>(nativeHandle);
		if (!handle) {
			ENGINE_WARN("No native handle set");
			return;
		}

		// UTF-16 buffer
		// Can't use u16string because Win32 expects wchar not char16
		std::wstring convBuffer;
		static_assert(sizeof(convBuffer[0]) == sizeof(char16_t), "Assumes a two byte wide char");

		const auto u8size = std::transform_reduce(array.cbegin(), array.cend(), 0ull, std::plus{},
			[](auto view) { return view.size(); }
		);

		convBuffer.resize(u8size); // u16 size will always be less than or equal to the u8 size

		// Convert to UTF-16
		size_t offset = 0; // Offset, in wide chars
		for (const auto& view : array) {
			ENGINE_DEBUG_ASSERT(!view.empty(), "It is not valid to set empty clipboard data. Use clearClipboard.");

			const auto len = ::MultiByteToWideChar(CP_UTF8, 0,
				view.data(),
				static_cast<int>(view.size()), // Size in bytes
				convBuffer.data() + offset,
				static_cast<int>(convBuffer.size()) // Size in wide chars
			);

			if (ENGINE_DEBUG && len == 0) {
				ENGINE_WARN("Unable to convert clipboard data: ", Win32::getLastErrorMessage());
			}

			offset += len;
		}

		// Discard any unused code units
		convBuffer.resize(std::wcslen(convBuffer.data()) + 1); // + 1 for null
		convBuffer.back() = 0;

		// See: https://learn.microsoft.com/en-us/windows/win32/dataxchg/using-the-clipboard
		const auto bytes = convBuffer.size() * 2;
		if (const auto mem = ::GlobalAlloc(GMEM_MOVEABLE, bytes)) {
			if (const auto ptr = ::GlobalLock(mem)) {
				memcpy(ptr, convBuffer.data(), bytes);
				::GlobalUnlock(mem);

				if (!::OpenClipboard(handle)) {
					ENGINE_WARN("Unable to open clipboard: ", Win32::getLastErrorMessage());
				} else {
					if (::EmptyClipboard() && ::SetClipboardData(CF_UNICODETEXT, mem)) {
						// Success, don't free mem
					} else {
						ENGINE_WARN("Unable to set clipboard data: ", Win32::getLastErrorMessage());
						::GlobalFree(mem);
					}

					if (!::CloseClipboard()) {
						ENGINE_WARN("Unable to close clipboard: ", Win32::getLastErrorMessage());
					}
				}
			} else {
				ENGINE_WARN("Unable to lock clipboard memory: ", Win32::getLastErrorMessage());
			}
		} else {
			ENGINE_WARN("Unable to allocate global memory for clipboard: ", Win32::getLastErrorMessage());
		}
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
			//auto target = getHover();

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

	bool Context::onText(std::string_view str, Input::KeyCode code) {
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
			if (panel->isEnabled() && cb(str, code)) { return true; }
		}

		return false;
	}

	void Context::onResize(const int32 w, const int32 h) {
		if (w == view.x && h == view.y) { return; }
		if (!(w||h)) { return; } // Minimize

		view = {w, h};
		root->setSize(view);
		colorTex.setStorage(Gfx::TextureFormat::RGBA8, view);
		glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, colorTex.get(), 0);
		resize(view);
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
