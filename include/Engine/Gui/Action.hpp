#pragma once


namespace Engine::Gui {
	enum class Action {
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
	};
}
