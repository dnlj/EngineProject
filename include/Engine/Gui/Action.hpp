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

		// TODO: "scroll" isnt an action... "zoom in", "zoom out", "move line up", etc are actions
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
