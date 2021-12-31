#pragma once

// STD
#include <ranges>

// GLM
#include <glm/vec2.hpp>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Gui/Layout.hpp>
#include <Engine/Gui/Bounds.hpp>
#include <Engine/Gui/Action.hpp>


namespace Engine::Gui {
	class Context; // Forward decl

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
		private:
			struct Flag_ {
				/** @see Panel states section */
				enum Flag : uint32 {
					Enabled          = 1 << 0,
					PerformingLayout = 1 << 1,
					_count,
				};
			};

		public:
			using Flag = Flag_::Flag;

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
			float32 weight = 1; // TODO: separte x,y weight?

			uint32 flags = Flag::Enabled;

			// TODO: maybe change to flag for diff size policies?
			bool autoSizeHeight = false;
			bool autoSizeWidth = false;

		public:
			Panel(Context* context) : ctx{context} {}
			virtual ~Panel();

			ENGINE_INLINE void setWeight(float32 w) noexcept { weight = w; }
			ENGINE_INLINE auto getWeight() const noexcept { return weight; }
			ENGINE_INLINE auto getParent() const noexcept { return parent; }

			ENGINE_INLINE void setAutoSizeHeight(bool v) noexcept { autoSizeHeight = v; }
			ENGINE_INLINE bool getAutoSizeHeight() const noexcept { return autoSizeHeight; }

			ENGINE_INLINE void setAutoSizeWidth(bool v) noexcept { autoSizeWidth = v; }
			ENGINE_INLINE bool getAutoSizeWidth() const noexcept { return autoSizeWidth; }

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
			virtual void render() const;

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
			ENGINE_INLINE void setEnabled(bool e) noexcept {
				const auto old = isEnabled();
				setFlag(Flag::Enabled, e);
				if (old != e && parent) { parent->onChildChanged(this); }
			}
			ENGINE_INLINE bool isEnabled() const noexcept { return getFlag(Flag::Enabled); }

			virtual void onAction(ActionEvent action) {}
			
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
			virtual void onBeginActivate() {}
			virtual void onEndActivate() {}

			/**
			 * Sets a Panel::Flag.
			 * You should probably be using specific setXYZ/isXYZ functions unless you have a
			 * good reason to use these. Could potentially modify internal flags and have
			 * unintended consequences.
			 */
			ENGINE_INLINE void setFlag(Flag f, bool e) noexcept { e ? (flags |= f) : (flags &= ~f); }
			ENGINE_INLINE bool getFlag(Flag f) const noexcept { return flags & f; }

		private:
			/**
			 * @see Flag
			 */
			ENGINE_INLINE void setPerformingLayout(bool e) noexcept { setFlag(Flag::PerformingLayout, e); }
			ENGINE_INLINE bool isPerformingLayout() const noexcept { return getFlag(Flag::PerformingLayout); }
			
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
}
