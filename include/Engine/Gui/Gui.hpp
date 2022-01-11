#pragma once


namespace Engine::Gui {
	struct Direction_ {
		enum Direction {
			Horizontal,
			Vertical,
			_count,
		};
	};
	using Direction = Direction_::Direction;

	struct Align_ {
		enum Align {
			Start,
			End,
			Center,
			Stretch,
			_count,
		};
	};
	using Align = Align_::Align;

	const char* getFreeTypeErrorString(const int err) noexcept;
}
