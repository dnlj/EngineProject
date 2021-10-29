#pragma once


namespace Engine::Gui {
	class Layout {
		public:
			virtual ~Layout() {}
			virtual void layout(class Panel* panel) = 0;
	};
}
