#pragma once

// GLM
#include <glm/vec2.hpp>

// Engine
#include <Engine/Engine.hpp>


namespace Engine::Gui {
	class Context; // Forward decl

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

			void render(Context& ctx) const;

			/**
			 * Called when this panel or any child panel are hovered.
			 * @return True to prevent this event from propagating to children.
			 */
			virtual bool onBeginHover(Panel* target) { return false; };
			virtual bool onEndHover(Panel* target) { return false; };

			/**
			 * Called when this panel or any child panel are focused.
			 * @return True to prevent this event from propagating to children.
			 */
			virtual bool onBeginFocus(Panel* target) { return false; };
			virtual bool onEndFocus(Panel* target) { return false; };

			/**
			 * Determines if this panel can gain focus.
			 */
			virtual bool canFocus() const { return false; }
			
			/**
			 * Called when this panel or any child panel is activated.
			 * @return True to prevent this event from propagating to children.
			 */
			// TODO: do we make this more generic? we could have more than simply mouse input
			virtual bool onBeginActivate() { return false; };
			virtual bool onEndActivate() { return false; };
	};	
}
