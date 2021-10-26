#pragma once

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
	 * TODO: doc
	 * 
	 * Panel States:
	 * - Hover: The panel is under the cursor.
	 * - Focus: The panel is the target of input. Ex: pressed with mouse, tab key navigation, text box input.
	 * - Active: The panel being actived. Ex: mouse press, enter key, controller X/A.
	 * - Default: No other state is present.
	 */
	class Panel {
		private:
			struct Flag_ {
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

			glm::vec2 minSize = {0, 0};
			glm::vec2 maxSize = {INFINITY, INFINITY};
			glm::vec2 idealSize;

			glm::vec2 pos;
			glm::vec2 size;

			uint32 flags = Flag::Enabled;

		public:
			Panel(Context* context) : ctx{context} {}
			virtual ~Panel();

			/**
			 * Remove a child from this panel.
			 * This does not delete the child. You now own the removed panel.
			 */
			void removeChild(Panel* child) {
				ENGINE_DEBUG_ASSERT(child->parent == this);

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
			}

			/**
			 * Add a child to the end of the child list.
			 * This panel now owns the child.
			 */
			auto addChild(Panel* child) {
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
				return child;
			}

			ENGINE_INLINE auto getParent() const noexcept { return parent; }
			
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
			 * Set the position of this panel relative to its parent.
			 */
			ENGINE_INLINE void setRelPos(const glm::vec2 p) noexcept { setPos(p + (parent ? parent->getPos() : glm::vec2{})); }
			ENGINE_INLINE auto getRelPos() const noexcept { return getPos() - (parent ? parent->getPos() : glm::vec2{}); }
			
			ENGINE_INLINE void setMinSize(const glm::vec2 sz) noexcept { minSize = sz; }
			ENGINE_INLINE auto getMinSize() const noexcept { return minSize; }
			
			ENGINE_INLINE void setMaxSize(const glm::vec2 sz) noexcept { maxSize = sz; }
			ENGINE_INLINE auto getMaxSize() const noexcept { return maxSize; }

			ENGINE_INLINE void updateParentPos(const glm::vec2 p) {
				const auto rel = getRelPos();
				setPos(p + rel);
			}

			/**
			 * Set the size of this panel.
			 * 
			 * Clamped to min/max size. \n
			 * May update panel layout.
			 */
			ENGINE_INLINE void setSize(const glm::vec2 sz) noexcept {
				const auto old = size;
				size = glm::clamp(sz, minSize, maxSize);
				if (size != old) { performLayout(); }
			};
			ENGINE_INLINE auto getSize() const noexcept { return size; }

			/**
			 * See @ref setSize.
			 */
			ENGINE_INLINE void setWidth(const float32 w) noexcept {
				const auto old = size.x;
				size.x = glm::clamp(w, minSize.x, maxSize.x);
				if (size.x != old) { performLayout(); }
			}
			ENGINE_INLINE auto getWidth() const noexcept { return size.x; }
			
			/**
			 * See @ref setSize.
			 */
			ENGINE_INLINE void setHeight(const float32 h) noexcept {
				const auto old = size.y;
				size.y = glm::clamp(h, minSize.y, maxSize.y);
				if (size.y != old) { performLayout(); }
			}
			ENGINE_INLINE auto getHeight() const noexcept { return size.y; }

			/**
			 * Gets the axis aligned bounding box for this panel.
			 */
			ENGINE_INLINE Bounds getBounds() const noexcept { return {pos, pos + size}; }

			ENGINE_INLINE void setBounds(Bounds bounds) noexcept {
				setPos(bounds.min);
				setSize(bounds.max - bounds.min);
			}

			// TODO: doc
			ENGINE_INLINE void setLayout(Layout* l) noexcept { layout = l; }
			ENGINE_INLINE auto getLayout() noexcept { return layout; }

			/**
			 * Renders this panel.
			 */
			virtual void render() const;

			// TODO: doc
			virtual void preLayout() {}
			virtual void postLayout() {}

			/**
			 * Called after a child panel has performed layout.
			 */
			void notifyLayout(Panel* child) {
				if (!isPerformingLayout()) {
					performLayout();
				}
			}

			// TODO: doc
			ENGINE_INLINE void performLayout() {
				preLayout();
				setPerformingLayout(true);
				if (layout) { layout->layout(this); }
				setPerformingLayout(false);
				postLayout();
				if (parent) { parent->notifyLayout(this); }
			}

			// TODO: doc
			/**
			 * 
			 */
			ENGINE_INLINE void setEnabled(bool e) noexcept { setFlag(Flag::Enabled, e); }
			ENGINE_INLINE bool isEnabled() const noexcept { return getFlag(Flag::Enabled); }

			virtual void onAction(Action act) {}
			
			/**
			 * Called when this panel is hovered.
			 */
			virtual void onBeginHover() { /*ENGINE_INFO("onBeginHover ", this); /**/ }
			virtual void onEndHover() { /*ENGINE_INFO("onEndHover ", this); /**/ }
			virtual bool canHover() const { return true; }
			
			/**
			 * Called the first time a child or any of its descendants are hovered.
			 * @param child The child that is, or is the parent of, the hovered panel.
			 */
			virtual void onBeginChildHover(Panel* child) { /*ENGINE_INFO("onBeginChildHover ", this, " ", child); /**/ }
			virtual void onEndChildHover(Panel* child) { /*ENGINE_INFO("onEndChildHover ", this, " ", child); /**/ }
			virtual bool canHoverChild(Panel* child) const { return true; }

			/**
			 * Called when this panel is focused.
			 */
			virtual void onBeginFocus() { /*ENGINE_INFO("onBeginFocus ", this); /**/ }
			virtual void onEndFocus() { /*ENGINE_INFO("onEndFocus ", this); /**/ }
			virtual bool canFocus() const { return true; }
			
			/**
			 * Called the first time a child or any of its descendants are focused.
			 * @param child The child that is, or is the parent of, the focused panel.
			 */
			virtual void onBeginChildFocus(Panel* child) { /*ENGINE_INFO("onBeginChildFocus ", this, " ", child); /**/ }
			virtual void onEndChildFocus(Panel* child) { /*ENGINE_INFO("onEndChildFocus ", this, " ", child); /**/ }
			virtual bool canFocusChild(Panel* child) const { return true; }

			/**
			 * Called when this panel is activated.
			 */
			virtual void onBeginActivate() {}
			virtual void onEndActivate() {}

		private:
			ENGINE_INLINE void setFlag(Flag f, bool e) noexcept { e ? (flags |= f) : (flags &= ~f); }
			ENGINE_INLINE bool getFlag(Flag f) const noexcept { return flags & f; }

			ENGINE_INLINE void setPerformingLayout(bool e) noexcept { setFlag(Flag::PerformingLayout, e); }
			ENGINE_INLINE bool isPerformingLayout() const noexcept { return getFlag(Flag::PerformingLayout); }
	};	
}
