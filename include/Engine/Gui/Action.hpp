#pragma once

// Engine
#include <Engine/Input/Value.hpp>


namespace Engine::Gui {
	enum class Action {
		Unknown,

		SelectBegin,
		SelectEnd,
		SelectAll,

		Cut,
		Copy,
		Paste,
		DeleteNext,
		DeletePrev,

		MoveCharLeft,
		MoveCharRight,
		MoveCharUp,
		MoveCharDown,
		
		MoveWordLeft,
		MoveWordRight,
		MoveWordUp,
		MoveWordDown,

		MoveLineStart,
		MoveLineEnd,

		PanelNext,
		PanelPrev,

		Scroll,  /** The number of lines to scroll - float32 */
		ScrollH, /** The number of chars to scroll - float32 */

		Submit,
	};

	class ActionEvent {
		public:
			ActionEvent(Action action = {}, Input::Value value = {})
				: action{action}, value{value} {
			}

			Action action;
			Input::Value value;

			operator Action() const noexcept { return action; }
	};
}
