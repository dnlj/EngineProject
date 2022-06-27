#pragma once

// STD
#include <ranges>

// GLM
#include <glm/vec2.hpp>

// Engine
#include <Engine/Engine.hpp>
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
		_count,
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
		public:

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

			ENGINE_INLINE auto getParent() const noexcept { return parent; }
			
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

			ENGINE_INLINE auto getNextSiblingRaw() const noexcept {
				return nextSibling;
			}

			/**
			 * Gets the next enabled sibling panel.
			 */
			ENGINE_INLINE auto getNextSibling() const noexcept {
				auto res = nextSibling;
				while (res && !res->isEnabled()) { res = res->nextSibling; }
				return (res && !res->isEnabled()) ? nullptr : res;
			}
			
			/**
			 * Gets the prev enabled sibling panel.
			 */
			ENGINE_INLINE auto getPrevSibling() const noexcept {
				auto res = prevSibling;
				while (res && !res->isEnabled()) { res = res->prevSibling; }
				return (res && !res->isEnabled()) ? nullptr : res;
			}

			ENGINE_INLINE auto getFirstChildRaw() const noexcept {
				return firstChild;
			}

			/**
			 * Gets the first enabled child panel.
			 */
			ENGINE_INLINE auto getFirstChild() const noexcept {
				return (firstChild && !firstChild->isEnabled()) ? firstChild->getNextSibling() : firstChild;
			}
			
			/**
			 * Gets the last enabled child panel.
			 */
			ENGINE_INLINE auto getLastChild() const noexcept {
				return (lastChild && !lastChild->isEnabled()) ? lastChild->getPrevSibling() : lastChild;
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
			
			ENGINE_INLINE void setMinSize(const glm::vec2 sz) noexcept { minSize = sz; setSize(getSize()); }
			ENGINE_INLINE auto getMinSize() const noexcept { return minSize; }
			
			ENGINE_INLINE void setMaxSize(const glm::vec2 sz) noexcept { maxSize = sz; setSize(getSize()); }
			ENGINE_INLINE auto getMaxSize() const noexcept { return maxSize; }

			ENGINE_INLINE void setFixedWidth(const float32 w) noexcept { minSize.x = w; maxSize.x = w; setWidth(w); }
			ENGINE_INLINE void setFixedHeight(const float32 h) noexcept { minSize.y = h; maxSize.y = h; setHeight(h); }
			ENGINE_INLINE void setFixedSize(const glm::vec2 sz) noexcept { minSize = sz; maxSize = sz; setSize(sz); }

			ENGINE_INLINE void lockWidth() noexcept { setFixedWidth(getWidth()); }
			ENGINE_INLINE void lockHeight() noexcept { setFixedHeight(getHeight()); }
			ENGINE_INLINE void lockSize() noexcept { setFixedSize(getSize()); }

			ENGINE_INLINE void updateParentPos(const glm::vec2 p) {
				const auto rel = getRelPos();
				setPos(p + rel);
			}

			ENGINE_INLINE auto getAutoHeight() const {
				if (layout) { return std::max(layout->getAutoHeight(this), minSize.y); }
				return getHeight();
			}

			ENGINE_INLINE void autoHeight() {
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
			 */
			void removeChild(Panel* child) {
				ENGINE_DEBUG_ASSERT(child->parent == this);
				child->setPos(child->getRelPos());

				if (child->nextSibling) {
					child->nextSibling->prevSibling = child->prevSibling;
				}

				if (child->prevSibling) {
					child->prevSibling->nextSibling = child->nextSibling;
				}

				if (child == lastChild) {
					lastChild = child->prevSibling;
				}

				if (child == firstChild) {
					firstChild = child->nextSibling;
				}

				child->prevSibling = nullptr;
				child->nextSibling = nullptr;
				child->parent = nullptr;

				performLayout();
			}

			/**
			 * Add a child to the end of the child list.
			 * This panel now owns the child.
			 */
			Panel* addChild(Panel* child) {
				appendChild(child);
				performLayout();
				return child;
			}

			/**
			 * Add multiple panels as children of this panel.
			 * Perfoms layout only once.
			 */
			void addChildren(std::initializer_list<Panel*> children) {
				for (Panel* child : children) {
					appendChild(child);
				}
				performLayout();
			}

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
			 */
			virtual void onBeginHover() { /*ENGINE_INFO("onBeginHover ", this); /* */ }
			virtual void onEndHover() { /*ENGINE_INFO("onEndHover ", this); /* */ }
			virtual bool canHover() const { return true; }
			
			/**
			 * Called the first time a child or any of its descendants are hovered.
			 * @param child The child that is, or is the parent of, the hovered panel.
			 */
			virtual void onBeginChildHover(Panel* child) { /*ENGINE_INFO("onBeginChildHover ", this, " ", child); /* */ }
			virtual void onEndChildHover(Panel* child) { /*ENGINE_INFO("onEndChildHover ", this, " ", child); /* */ }
			virtual bool canHoverChild(Panel* child) const { return true; }

			/**
			 * Called when this panel is focused.
			 */
			virtual void onBeginFocus() { /*ENGINE_INFO("onBeginFocus ", this); /* */ }
			virtual void onEndFocus() { /*ENGINE_INFO("onEndFocus ", this); /* */ }
			virtual bool canFocus() const { return true; }
			
			/**
			 * Called the first time a child or any of its descendants are focused.
			 * @param child The child that is, or is the parent of, the focused panel.
			 */
			virtual void onBeginChildFocus(Panel* child) { /*ENGINE_INFO("onBeginChildFocus ", this, " ", child); /* */ }
			virtual void onEndChildFocus(Panel* child) { /*ENGINE_INFO("onEndChildFocus ", this, " ", child); /* */ }
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

			/**
			 * Sets a panel as a child of this panel.
			 * Does not perform layout.
			 */
			Panel* appendChild(Panel* child) {
				if (child->parent) {
					child->parent->removeChild(child);
				}

				if (lastChild) {
					ENGINE_DEBUG_ASSERT(lastChild->nextSibling == nullptr);
					ENGINE_DEBUG_ASSERT(child->prevSibling == nullptr);
					lastChild->nextSibling = child;
					child->prevSibling = lastChild;
				} else {
					ENGINE_DEBUG_ASSERT(firstChild == nullptr);
					firstChild = child;
				}

				child->parent = this;
				lastChild = child;
				child->setRelPos(child->getPos());
				return child;
			}
	};

	/**
	 * A transparent panel.
	 * Typeically used for non-visible organization and clipping.
	 */
	class PanelT : public Panel {
		using Panel::Panel;
		virtual void render() override {}
	};
}
