#pragma once

// GLM
#include <glm/vec2.hpp>

// Engine
#include <Engine/Engine.hpp>


namespace Engine::Gui {
	class Context; // Forward decl

	class Bounds { // TODO: move
		public:
			glm::vec2 topLeft;
			glm::vec2 bottomRight;

			ENGINE_INLINE bool contains(const glm::vec2 point) const noexcept {
				return point.x >= topLeft.x
					&& point.y >= topLeft.y
					&& point.x <= bottomRight.x
					&& point.y <= bottomRight.y;
			}

			ENGINE_INLINE friend Bounds operator+(const Bounds& a, const glm::vec2 b) noexcept {
				auto res = a;
				res.topLeft += b;
				res.bottomRight += b;
				return res;
			}
	};

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
			Panel* parent = nullptr;
			Panel* prevSibling = nullptr;
			Panel* nextSibling = nullptr;
			Panel* firstChild = nullptr;
			Panel* lastChild = nullptr;

			glm::vec2 minSize = {0, 0};
			glm::vec2 maxSize = {INFINITY, INFINITY};
			glm::vec2 idealSize;

			glm::vec2 pos;
			glm::vec2 size;

			bool enabled = true;

		public:
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
				while (res && !res->enabled) { res = res->nextSibling; }
				return (res && !res->enabled) ? nullptr : res;
			}

			/**
			 * Gets the first enabled child panel.
			 */
			ENGINE_INLINE auto getFirstChild() const noexcept {
				return (firstChild && !firstChild->enabled) ? firstChild->getNextSibling() : firstChild;
			}

			// TODO: doc
			/**
			 *
			 */
			ENGINE_INLINE void setEnabled(bool e) noexcept { enabled = e; }
			ENGINE_INLINE auto getEnabled() const noexcept { return enabled; }

			/**
			 * Set the absolute position of this panel.
			 */
			ENGINE_INLINE void setPos(const glm::vec2 p) noexcept { pos = p; }
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

			/**
			 * Set the size of this panel.
			 * 
			 * Clamped to min/max size. \n
			 * May update panel layout.
			 */
			ENGINE_INLINE void setSize(const glm::vec2 sz) noexcept {
				const auto old = size;
				size = glm::clamp(sz, minSize, maxSize);
				if (size != old) { layout(); }
			};
			ENGINE_INLINE auto getSize() const noexcept { return size; }

			/**
			 * See @ref setSize.
			 */
			ENGINE_INLINE void setWidth(const float32 w) noexcept {
				const auto old = size.x;
				size.x = glm::clamp(w, minSize.x, maxSize.x);
				if (size.x != old) { layout(); }
			}
			ENGINE_INLINE auto getWidth() const noexcept { return size.x; }
			
			/**
			 * See @ref setSize.
			 */
			ENGINE_INLINE void setHeight(const float32 h) noexcept {
				const auto old = size.y;
				size.y = glm::clamp(h, minSize.y, maxSize.y);
				if (size.y != old) { layout(); }
			}
			ENGINE_INLINE auto getHeight() const noexcept { return size.y; }

			/**
			 * Gets the axis aligned bounding box for this panel.
			 */
			ENGINE_INLINE Bounds getBounds() const noexcept { return {pos, pos + size}; }

			/**
			 * Renders this panel.
			 */
			virtual void render(Context& ctx) const;

			// TODO: doc
			virtual void layout() {}
			
			/**
			 * Called when this panel is hovered.
			 */
			virtual void onBeginHover() { /*ENGINE_INFO("onBeginHover ", this); /**/ };
			virtual void onEndHover() { /*ENGINE_INFO("onEndHover ", this); /**/ };
			virtual bool canHover() const { return true; }
			
			/**
			 * Called the first time a child or any of its descendants are hovered.
			 * @param child The child that is, or is the parent of, the hovered panel.
			 */
			virtual void onBeginChildHover(Panel* child) { /*ENGINE_INFO("onBeginChildHover ", this, " ", child); /**/ };
			virtual void onEndChildHover(Panel* child) { /*ENGINE_INFO("onEndChildHover ", this, " ", child); /**/ };
			virtual bool canHoverChild(Panel* child) const { return true; }

			/**
			 * Called when this panel is focused.
			 */
			virtual void onBeginFocus() { /*ENGINE_INFO("onBeginFocus ", this); /**/ };
			virtual void onEndFocus() { /*ENGINE_INFO("onEndFocus ", this); /**/ };
			virtual bool canFocus() const { return true; }
			
			/**
			 * Called the first time a child or any of its descendants are focused.
			 * @param child The child that is, or is the parent of, the focused panel.
			 */
			virtual void onBeginChildFocus(Panel* child) { /*ENGINE_INFO("onBeginChildFocus ", this, " ", child); /**/ };
			virtual void onEndChildFocus(Panel* child) { /*ENGINE_INFO("onEndChildFocus ", this, " ", child); /**/ };
			virtual bool canFocusChild(Panel* child) const { return true; }

			/**
			 * Called when this panel is activated.
			 */
			virtual void onBeginActivate() {};
			virtual void onEndActivate() {};
	};	
}
