#pragma once


namespace Engine::Gui {
	class Panel {
		private:
			Panel* parent = nullptr;
			Panel* nextSibling = nullptr;
			Panel* firstChild = nullptr;
			Panel* lastChild = nullptr;

		public:
			virtual ~Panel() {};

			void addChild(Panel* child);

			void doLayout();

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
