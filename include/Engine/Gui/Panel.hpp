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

	// TODO: doc states (focus, hover, active)
	/**
	 *
	 */
	class Panel {
		friend class Context;
		private:
			Panel* parent = nullptr;
			Panel* nextSibling = nullptr;
			Panel* firstChild = nullptr;
			Panel* lastChild = nullptr;

			glm::vec2 minSize = {0, 0};
			glm::vec2 maxSize = {INFINITY, INFINITY};
			glm::vec2 idealSize;

			glm::vec2 pos;
			glm::vec2 size;

		public:
			virtual ~Panel();

			auto addChild(Panel* child) {
				if (lastChild) {
					ENGINE_DEBUG_ASSERT(lastChild->nextSibling == nullptr);
					lastChild->nextSibling = child;
				} else {
					ENGINE_DEBUG_ASSERT(firstChild == nullptr);
					firstChild = child;
				}

				child->parent = this;
				lastChild = child;
				return child;
			}

			ENGINE_INLINE void setPos(const glm::vec2 p) noexcept { pos = p; }
			ENGINE_INLINE auto getPos() const noexcept { return pos; }

			ENGINE_INLINE void setMinSize(const glm::vec2 sz) noexcept { minSize = sz; }
			ENGINE_INLINE auto getMinSize() const noexcept { return minSize; }

			ENGINE_INLINE void setMaxSize(const glm::vec2 sz) noexcept { maxSize = sz; }
			ENGINE_INLINE auto getMaxSize() const noexcept { return maxSize; }

			ENGINE_INLINE void setSize(const glm::vec2 sz) noexcept { size = glm::clamp(sz, minSize, maxSize); };
			ENGINE_INLINE auto getSize() const noexcept { return size; }

			/**
			 * Gets the axis aligned bounding box for this panel.
			 */
			ENGINE_INLINE Bounds getBounds() const noexcept { return {pos, pos + size}; }

			virtual void render(Context& ctx) const;
			
			/**
			 * Called when this panel is hovered.
			 */
			virtual void onBeginHover() { /*ENGINE_LOG("onBeginHover ", this); /**/};
			virtual void onEndHover() { /*ENGINE_LOG("onEndHover ", this); /**/};
			virtual bool canHover() const { return true; }
			
			/**
			 * Called when any descendant panel is hovered.
			 * @param child The direct child that is, or is a parent of, the hovered panel.
			 * @return True to intercept this event and prevent it from propagating to children.
			 */
			virtual bool onBeginChildHover(Panel* child) { /*ENGINE_LOG("onBeginChildHover ", this, " ", child); /**/ return false;};
			virtual void onEndChildHover(Panel* child) { /*ENGINE_LOG("onEndChildHover ", this, " ", child); /**/};

			/**
			 * Called when this panel is focused.
			 */
			virtual void onBeginFocus() { ENGINE_INFO("onBeginFocus ", this); };
			virtual void onEndFocus() { ENGINE_INFO("onEndFocus ", this); };
			virtual bool canFocus() const { return true; }

			// TODO: doc
			virtual void onBeginChildFocus(Panel* child) { ENGINE_INFO("onBeginChildFocus ", this, " ", child); };
			virtual void onEndChildFocus(Panel* child) { ENGINE_INFO("onEndChildFocus ", this, " ", child); };
			virtual bool canFocusChild(Panel* child) { return true; }

			/**
			 * Called when this panel or any child panel is activated.
			 * @return True to prevent this event from propagating to children.
			 */
			// TODO: do we make this more generic? we could have more than simply mouse input
			virtual bool onBeginActivate() { return false; };
			virtual bool onEndActivate() { return false; };
	};	
}
