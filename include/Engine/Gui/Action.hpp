#pragma once


namespace Engine::Gui {
	enum class Action {
		CursorSelectBegin,
		CursorSelectEnd,

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

		PanelNext,
		PanelPrev,
	};
}
