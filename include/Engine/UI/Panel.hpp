#pragma once

// GLM
#include <glm/vec2.hpp>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/ArrayView.hpp>
#include <Engine/UI/Layout.hpp>
#include <Engine/UI/Bounds.hpp>
#include <Engine/UI/Action.hpp>


namespace Engine::UI {
	class Context; // Forward decl

	/** @see Panel states section */
	enum class PanelState : uint32 {
		Enabled          = 1 << 0,
		PerformingLayout = 1 << 1,
		ParentEnabled    = 1 << 2,
		Deleted			 = 1 << 3,
	}; ENGINE_BUILD_ALL_OPS(PanelState);

	/**
	 * The generic interface for all UI elements.
	 * 
	 * States:
	 * ===========================================================================
	 * - Hover: The panel is under the cursor.
	 * - Focus: The panel is the target of input. Ex: pressed with mouse, tab key navigation, text box input.
	 * - Active: The panel being actived. Ex: mouse press, enter key, controller X/A.
	 * - Default: No other state is present.
	 * - Enabled: When a panel is not enabled it is removed from all calculations, layout, iteration, etc.
	 *            It still exists, is a child of its parent, and owns its children, but it is skipped during
	 *            sibling/child iteration. The children of a disabled panel are also disabled.
	 * - PerformingLayout: The panel is updating its layout.
	 */
	class Panel {
		protected:
			/** The Context that owns this panel. */
			Context* ctx = nullptr;

		private:
			Panel* parent = nullptr;
			Panel* prevSibling = nullptr;
			Panel* nextSibling = nullptr;
			Panel* firstChild = nullptr;
			Panel* lastChild = nullptr;
			Layout* layout = nullptr;

			glm::vec2 minSize = {8, 8};
			glm::vec2 maxSize = {INFINITY, INFINITY};
			glm::vec2 idealSize = {};

			glm::vec2 pos = {};
			glm::vec2 size = minSize;

			/**
			 * Relative weights for various panel layouts.
			 * Zero indicates no change/automatic weight.
			 */
			float32 weight = 1; // TODO: separate x,y weight

			glm::ivec2 gridPos = {}; // TODO: is there a better way to handle these layout specific properties?

			PanelState flags = PanelState::Enabled | PanelState::ParentEnabled;

			// TODO: maybe change to flag for diff size policies?
			bool autoSizeHeight = false;
			bool autoSizeWidth = false;

		public:
			/**
			 * Builds the sibling chain for a group of panels.
			 * 
			 * These should probably be orphaned panels because this does not modify
			 * any other relationships or update related panels.
			 * You probably want addChildren or insertChildren instead.
			 * 
			 * @see addChildren
			 * @see insertChildren
			 */
			static void unsafe_CreateSiblings(ArrayView<Panel* const> children) noexcept {
				ENGINE_DEBUG_ASSERT(children.size() > 1, "Attempting to create siblings out of one or fewer panels.");
				auto curr = children.begin();
				auto next = curr + 1;
				const auto end = children.end();

				ENGINE_DEBUG_ASSERT((*curr)->parent == nullptr);
				while (next != end) {
					ENGINE_DEBUG_ASSERT((*next)->parent == nullptr);
					(*curr)->nextSibling = *next;
					(*next)->prevSibling = *curr;
					curr = next;
					++next;
				}
			}

			/**
			 * Removes children from a parents sibling chain.
			 * Does NOT update the removed childrens parent relation or adjust position.
			 * You probably want removeChild or removeChildren instead
			 *
			 * @see removeChild
			 * @see removeChildren
			 */
			static void unsafe_orphanChildren(Panel* parent, Panel* first, Panel* last) noexcept {
				if (last->nextSibling) {
					last->nextSibling->prevSibling = first->prevSibling;
				}

				if (first->prevSibling) {
					first->prevSibling->nextSibling = last->nextSibling;
				}

				if (last == parent->lastChild) {
					parent->lastChild = first->prevSibling;
				}

				if (first == parent->firstChild) {
					parent->firstChild = last->nextSibling;
				}

				first->prevSibling = nullptr;
				last->nextSibling = nullptr;
			}

			/**
			 * Directly sets a panels parent.
			 * Does NOT update any other relations.
			 * You probably want addChildren or insertChildren instead.
			 * 
			 * @see addChildren
			 * @see insertChildren
			 */
			static void unsafe_setParent(Panel* child, Panel* parent) noexcept {
				child->parent = parent;
			}

			/**
			 * Sets a panels state to a deleted state.
			 * Does NOT update any child panels.
			 * You may want setEnabled instead.
			 *
			 * @see setEnabled
			 */
			ENGINE_INLINE static void unsafe_markDeleted(Panel* panel) noexcept {
				// Set PerformingLayout in case we some how call layout on a deleted panel.
				// In debug we have warnings for this, but could happen I guess. Just a safety net for release mode.
				panel->flags = PanelState::Deleted | PanelState::PerformingLayout;
			}

			/**
			 * Directly modifies a panels flags. You should use specific
			 * Panel::set* functions instead. Calling this will put your panels
			 * in an invalid and unsupported state.
			 */
			ENGINE_INLINE static void unsafe_setFlag(Panel* panel, PanelState f, bool e) noexcept { e ? (panel->flags |= f) : (panel->flags &= ~f); }

		public:
			Panel(Context* context) : ctx{context} {}
			Panel(Panel&) = delete;
			virtual ~Panel();

			/**
			 * The column and row this panel resides in if using a grid/table/ordered layout.
			 */
			ENGINE_INLINE void setGridPos(int32 col, int32 row) noexcept { gridPos = {col, row}; }
			ENGINE_INLINE auto getGridPos() const noexcept { return gridPos; }
			
			/** @see setGridPos */
			ENGINE_INLINE void setGridColumn(int32 col) noexcept { gridPos.x = col; }
			ENGINE_INLINE auto getGridColumn() const noexcept { return gridPos.x; }
			
			/** @see setGridPos */
			ENGINE_INLINE void setGridRow(int32 row) noexcept { gridPos.y = row; }
			ENGINE_INLINE auto getGridRow() const noexcept { return gridPos.y; }

			/**
			 * The relative weight for this panel if using a weighted layout.
			 */
			ENGINE_INLINE void setWeight(float32 w) noexcept { weight = w; }
			ENGINE_INLINE auto getWeight() const noexcept { return weight; }

			ENGINE_INLINE Panel* getParent() const noexcept { return parent; }
			
			/** @see setAutoSize */
			ENGINE_INLINE void setAutoSizeWidth(bool v) noexcept { autoSizeWidth = v; }
			ENGINE_INLINE bool getAutoSizeWidth() const noexcept { return autoSizeWidth; }

			/** @see setAutoSize */
			ENGINE_INLINE void setAutoSizeHeight(bool v) noexcept { autoSizeHeight = v; }
			ENGINE_INLINE bool getAutoSizeHeight() const noexcept { return autoSizeHeight; }

			/**
			 * Set if this panel should resize its width/height to fit child panels.
			 */
			ENGINE_INLINE void setAutoSize(bool v) noexcept { setAutoSizeWidth(v); setAutoSizeHeight(v); }

			/**
			 * Gets the next enabled sibling panel.
			 */
			ENGINE_INLINE auto getNextSibling() const noexcept {
				auto res = nextSibling;
				while (res && !res->isEnabled()) { res = res->nextSibling; }
				return (res && !res->isEnabled()) ? nullptr : res;
			}

			ENGINE_INLINE auto getNextSiblingRaw() const noexcept {
				return nextSibling;
			}
			
			/**
			 * Gets the prev enabled sibling panel.
			 */
			ENGINE_INLINE auto getPrevSibling() const noexcept {
				auto res = prevSibling;
				while (res && !res->isEnabled()) { res = res->prevSibling; }
				return (res && !res->isEnabled()) ? nullptr : res;
			}

			ENGINE_INLINE auto getPrevSiblingRaw() const noexcept {
				return prevSibling;
			}

			/**
			 * Gets the first enabled child panel.
			 */
			ENGINE_INLINE auto getFirstChild() const noexcept {
				return (firstChild && !firstChild->isEnabled()) ? firstChild->getNextSibling() : firstChild;
			}

			ENGINE_INLINE auto getFirstChildRaw() const noexcept {
				return firstChild;
			}
			
			/**
			 * Gets the last enabled child panel.
			 */
			ENGINE_INLINE auto getLastChild() const noexcept {
				return (lastChild && !lastChild->isEnabled()) ? lastChild->getPrevSibling() : lastChild;
			}

			ENGINE_INLINE auto getLastChildRaw() const noexcept {
				return lastChild;
			}

			ENGINE_INLINE auto getContext() const noexcept { return ctx; }

			/**
			 * Set the absolute position of this panel.
			 */
			void setPos(const glm::vec2 p) noexcept {
				auto curr = firstChild;
				while (curr) {
					curr->updateParentPos(p);
					curr = curr->nextSibling;
				}
				pos = p;
			}
			ENGINE_INLINE auto getPos() const noexcept { return pos; }

			ENGINE_INLINE void setPosX(const float32 x) noexcept { setPos({x, pos.y}); }
			ENGINE_INLINE auto getPosX() const noexcept { return pos.x; }

			ENGINE_INLINE void setPosY(const float32 y) noexcept { setPos({pos.x, y}); }
			ENGINE_INLINE auto getPosY() const noexcept { return pos.y; }

			/**
			 * Set the size of this panel.
			 * 
			 * Clamped to min/max size. \n
			 * May update panel layout.
			 */
			void setSize(const glm::vec2 sz) noexcept {
				const auto old = size;
				size = glm::clamp(sz, minSize, maxSize);
				if (size != old) { sizeChanged(); }
			};
			ENGINE_INLINE auto getSize() const noexcept { return size; }

			/**
			 * See @ref setSize.
			 */
			void setWidth(const float32 w) noexcept {
				const auto old = size.x;
				size.x = glm::clamp(w, minSize.x, maxSize.x);
				if (size.x != old) { sizeChanged(); }
			}
			ENGINE_INLINE float32 getWidth() const noexcept { return size.x; }
			
			/**
			 * See @ref setSize.
			 */
			void setHeight(const float32 h) noexcept {
				const auto old = size.y;
				size.y = glm::clamp(h, minSize.y, maxSize.y);
				if (size.y != old) { sizeChanged(); }
			}
			ENGINE_INLINE float32 getHeight() const noexcept { return size.y; }

			/**
			 * Gets the axis aligned bounding box for this panel.
			 */
			ENGINE_INLINE Bounds getBounds() const noexcept { return {pos, pos + size}; }

			ENGINE_INLINE void setBounds(Bounds bounds) noexcept {
				setPos(bounds.min);
				setSize(bounds.max - bounds.min);
			}

			/**
			 * Set the position of this panel relative to its parent.
			 */
			ENGINE_INLINE void setRelPos(const glm::vec2 p) noexcept { setPos(p + (parent ? parent->getPos() : glm::vec2{})); }
			ENGINE_INLINE auto getRelPos() const noexcept { return getPos() - (parent ? parent->getPos() : glm::vec2{}); }

			ENGINE_INLINE void setMinWidth(float32 w) noexcept { setMinSize({w, minSize.y}); }
			ENGINE_INLINE void setMinHeight(float32 h) noexcept { setMinSize({minSize.x, h}); }
			ENGINE_INLINE void setMinSize(const glm::vec2 sz) noexcept { minSize = sz; setSize(getSize()); }

			ENGINE_INLINE auto getMinSize() const noexcept { return minSize; }
			ENGINE_INLINE auto getMinWidth() const noexcept { return getMinSize().x; }
			ENGINE_INLINE auto getMinHeight() const noexcept { return getMinSize().y; }
			
			ENGINE_INLINE void setMaxWidth(float32 w) noexcept { setMaxSize({w, maxSize.y}); }
			ENGINE_INLINE void setMaxHeight(float32 h) noexcept { setMaxSize({maxSize.x, h}); }
			ENGINE_INLINE void setMaxSize(const glm::vec2 sz) noexcept { maxSize = sz; setSize(getSize()); }

			ENGINE_INLINE auto getMaxSize() const noexcept { return maxSize; }
			ENGINE_INLINE auto getMaxWidth() const noexcept { return getMaxSize().x; }
			ENGINE_INLINE auto getMaxHeight() const noexcept { return getMaxSize().y; }

			ENGINE_INLINE void setFixedWidth(const float32 w) noexcept { minSize.x = w; maxSize.x = w; setWidth(w); }
			ENGINE_INLINE void setFixedHeight(const float32 h) noexcept { minSize.y = h; maxSize.y = h; setHeight(h); }
			ENGINE_INLINE void setFixedSize(const glm::vec2 sz) noexcept { minSize = sz; maxSize = sz; setSize(sz); }

			ENGINE_INLINE void lockWidth() noexcept { setFixedWidth(getWidth()); }
			ENGINE_INLINE void lockHeight() noexcept { setFixedHeight(getHeight()); }
			ENGINE_INLINE void lockSize() noexcept { setFixedSize(getSize()); }

			void center() noexcept {
				ENGINE_DEBUG_ASSERT(parent, "Attempting to center parentless panel.");
				setPos(parent->getPos() + (parent->getSize() - getSize()) * 0.5f);
			}

			ENGINE_INLINE void updateParentPos(const glm::vec2 p) {
				const auto rel = getRelPos();
				setPos(p + rel);
			}

			ENGINE_INLINE auto getAutoHeight() const {
				if (layout) { return std::max(layout->getAutoHeight(this), minSize.y); }
				return getHeight();
			}

			void autoHeight() {
				setHeight(getAutoHeight());
			}

			ENGINE_INLINE auto getAutoWidth() const {
				if (layout) { return std::max(layout->getAutoWidth(this), minSize.x); }
				return getHeight();
			}

			ENGINE_INLINE void autoWidth() {
				setWidth(getAutoWidth());
			}

			/**
			 * Sets the layout for this panel to use.
			 */
			ENGINE_INLINE void setLayout(Layout* l) noexcept {
				if (layout) { delete layout; }
				layout = l;
				performLayout();
			}
			ENGINE_INLINE auto getLayout() noexcept { return layout; }

			/**
			 * Remove a child from this panel.
			 * This does not delete the child. You now own the removed panel.
			 * 
			 * @see removeChildren
			 */
			ENGINE_INLINE void removeChild(Panel* child) {
				return removeChildren(child, child);
			}

			/**
			 * Removes the children in range [first, last] inclusive.
			 * This does not delete the children. You now own the removed panels.
			 * 
			 * @see removeChild
			 */
			ENGINE_INLINE void removeChildren(Panel* first, Panel* last) {
				removeChildrenNoLayout(first, last);
				performLayout();
			}

			/**
			 * Add a child to the end of the child list.
			 * This panel now owns the child.
			 * 
			 * @see addChildren
			 */
			ENGINE_INLINE Panel* addChild(Panel* child) {
				insertChildren(nullptr, child, child);
				return child;
			}

			/**
			 * Add multiple orphaned panels as children of this panel.
			 * Perfoms layout only once.
			 *
			 * @see addChild
			 */
			void addChildren(ArrayView<Panel* const> children) {
				if (children.size() > 1) {
					unsafe_CreateSiblings(children);
				}

				insertChildren(nullptr, children.front(), children.back());
			}

			/**
			 * Inserts the panel @p child before @p before.
			 *
			 * @see insertChildren
			 */
			ENGINE_INLINE void insertChild(Panel* before, Panel* child) {
				insertChildren(before, child, child);
			}

			/**
			 * Inserts the panels [@p first, @p last] inclusive before @p before.
			 * @p before Must not be in the range [@p first, @p last].
			 *
			 * @see insertChild
			 */
			ENGINE_INLINE void insertChildren(Panel* before, Panel* first, Panel* last) {
				insertChildrenNoLayout(before, first, last);
				performLayout();
				// TODO (eF1Tp1nZ): we also need to update the context state before this: active/hover/focus
			}

			/**
			 * Move this child to the top (last child).
			 */
			void toTop() { getParent()->addChild(this); }

			/**
			 * Renders this panel.
			 */
			virtual void render();

			/**
			 * Called before layout is performed.
			 */
			virtual void preLayout() {}

			/**
			 * Called after layout is performed.
			 */
			virtual void postLayout() {}

			/**
			 * Causes this panel to update its layout.
			 */
			ENGINE_INLINE void performLayout() {
				ENGINE_DEBUG_ASSERT(!isDeleted(), "Attempting to perform layout on a panel marked for deletion.");
				// TODO: We should also return if the panel is not enabled right?
				if (isPerformingLayout()) { return; }

				setPerformingLayout(true);
				preLayout();
				if (autoSizeHeight) { autoHeight(); }
				if (autoSizeWidth) { autoWidth(); }
				if (layout) { layout->layout(this); }
				postLayout();
				setPerformingLayout(false);
			}

			/**
			 * @see Flag
			 */
			void setEnabled(bool e) noexcept {
				const auto old = isEnabled();
				setFlag(PanelState::Enabled, e);
				if (old != e && parent) {
					for (auto child = firstChild; child;) {
						child->setParentEnabled(e);
						child = child->nextSibling;
					}
					parent->onChildChanged(this);
				}
			}
			ENGINE_INLINE bool isEnabled() const noexcept { return isFlag(PanelState::Enabled | PanelState::ParentEnabled); }
			ENGINE_INLINE void toggleEnabled() noexcept { setEnabled(!isEnabled()); }

			ENGINE_INLINE bool isDeleted() const noexcept { return isFlag(PanelState::Deleted); }

			void setParentEnabled(bool e) noexcept {
				const auto old = isEnabled();
				setFlag(PanelState::ParentEnabled, e);
				e = isEnabled();
				if (e != old) {
					for (auto child = firstChild; child;) {
						child->setParentEnabled(isEnabled());
						child = child->nextSibling;
					}
				}
			}

			/**
			 * Called when an ActionEvent is issued to this panel.
			 * If the event is not consumed then it is propagated up the parent chain.
			 * @return Was the event consumed.
			 */
			virtual bool onAction(ActionEvent action) { return false; }
			
			/**
			 * Called when this panel is hovered.
			 * If you cannot hover a panel you still can hover it's children.
			 * @see canHoverChild
			 */
			virtual void onBeginHover() { /*ENGINE_INFO("onBeginHover ", this); /* */ }
			virtual void onEndHover() { /*ENGINE_INFO("onEndHover ", this); /* */ }
			virtual bool canHover() const { return true; }
			
			/**
			 * Called the first time a child or any of its descendants are hovered.
			 * @param child The child that is, or is the parent of, the hovered panel.
			 */
			virtual void onBeginChildHover(Panel* child) { ENGINE_DEBUG_ASSERT(child->parent == this); /*ENGINE_INFO("onBeginChildHover ", this, " ", child); /* */ }
			virtual void onEndChildHover(Panel* child) { ENGINE_DEBUG_ASSERT(child->parent == this); /*ENGINE_INFO("onEndChildHover ", this, " ", child); /* */ }
			virtual bool canHoverChild(Panel* child) const { return true; }

			/**
			 * Called when this panel is focused.
			 * If you cannot focus a panel you still can focus it's children.
			 * @see canFocusChild
			 */
			virtual void onBeginFocus() { /*ENGINE_INFO("onBeginFocus ", this); /* */ }
			virtual void onEndFocus() { /*ENGINE_INFO("onEndFocus ", this); /* */ }
			virtual bool canFocus() const { return true; }
			
			/**
			 * Called the first time a child or any of its descendants are focused.
			 * @param child The child that is, or is the parent of, the focused panel.
			 */
			virtual void onBeginChildFocus(Panel* child) { ENGINE_DEBUG_ASSERT(child->parent == this); /*ENGINE_INFO("onBeginChildFocus ", this, " ", child); /* */ }
			virtual void onEndChildFocus(Panel* child) { ENGINE_DEBUG_ASSERT(child->parent == this); /*ENGINE_INFO("onEndChildFocus ", this, " ", child); /* */ }
			virtual bool canFocusChild(Panel* child) const { return true; }

			/**
			 * Called when this panel is activated.
			 */
			virtual bool onBeginActivate() { return true; }
			virtual void onEndActivate() {}

		private:
			/**
			 * Sets a flag.
			 * You should probably be using specific setXYZ/isXYZ functions unless you have a
			 * good reason to use these. Could potentially modify internal flags and have
			 * unintended consequences.
			 */
			ENGINE_INLINE void setFlag(PanelState f, bool e) noexcept { e ? (flags |= f) : (flags &= ~f); }
			ENGINE_INLINE auto getFlag2(PanelState f) const noexcept { return flags & f; }
			ENGINE_INLINE bool isFlag(PanelState f) const noexcept { return (flags & f) == f; }

			/**
			 * @see Flag
			 */
			ENGINE_INLINE void setPerformingLayout(bool e) noexcept { setFlag(PanelState::PerformingLayout, e); }
			ENGINE_INLINE bool isPerformingLayout() const noexcept { return isFlag(PanelState::PerformingLayout); }
			
			/**
			 * Called after a child panel has changed in a way that may affect the parent.
			 * Ex: size changed, enabled, disabled
			 */
			void onChildChanged(Panel* child) {
				performLayout();
			}

			/**
			 * Notify the parent panel that this panels size has changed.
			 */
			ENGINE_INLINE void sizeChanged() {
				performLayout();
				if (parent) { parent->onChildChanged(this); }
			}
			
			void insertChildrenNoLayout(Panel* before, Panel* first, Panel* last) {
				ENGINE_DEBUG_ASSERT(first != nullptr);
				ENGINE_DEBUG_ASSERT(last != nullptr);

				// Orphan children
				if (auto parent = first->parent) {
					if (parent == this) {
						removeChildrenNoLayout(first, last);
					} else {
						parent->removeChildren(first, last);
					}
				}

				// Insert children
				if (before == firstChild) {
					firstChild = first;
				}

				if (before == nullptr) {
					if (lastChild) {
						lastChild->nextSibling = first;
						first->prevSibling = lastChild;
					}

					lastChild = last;
				} else {
					if (auto prev = before->prevSibling) {
						prev->nextSibling = first;
						first->prevSibling = prev;
					}

					last->nextSibling = before;
					before->prevSibling = last;
				}

				// Make relative
				for (auto curr = first;; curr = curr->getNextSiblingRaw()) {
					curr->parent = this;
					// TODO: setPos calls layout on children, is there any way to defer this until after perform layout?
					// TODO: cont. because performLayout is likely(but not guaranteed) to move them again.
					curr->setPos(curr->getPos() + getPos()); // Update position to be relative to parent
					if (curr == last) { break; }
				}
			}

			void removeChildrenNoLayout(Panel* first, Panel* last) {
				ENGINE_DEBUG_ASSERT(first->parent == this, "Attempting to remove invalid panel range.");
				ENGINE_DEBUG_ASSERT(last->parent == this, "Attempting to remove invalid panel range.");

				ENGINE_INLINE_CALLS unsafe_orphanChildren(this, first, last);

				for (auto curr = first;; curr = curr->nextSibling) {
					curr->setPos(curr->getRelPos());
					curr->parent = nullptr;
					if (curr == last) { break; }
				}
			}
	};

	/**
	 * A transparent panel.
	 * Typeically used for non-visible organization.
	 */
	class PanelT : public Panel {
		using Panel::Panel;
		virtual void render() override {}
	};

	/**
	 * A panel with a colored background.
	 * Used for debugging.
	 */
	class PanelC final : public Panel {
		public:
			glm::vec4 color = {1,0,1,1};
			PanelC(Context* context, glm::vec4 color = {1,0,1,1}) : Panel{context}, color{color} {}
			virtual void render() override;
	};
}
