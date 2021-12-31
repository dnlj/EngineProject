#pragma once


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

		Scroll,
		ScrollH,
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
