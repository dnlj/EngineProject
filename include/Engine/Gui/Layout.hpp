#pragma once


namespace Engine::Gui {
	class Layout {
		public:
			virtual ~Layout() {}
			virtual void autoHeight(class Panel* panel) {};
			virtual void layout(class Panel* panel) = 0;
	};
}
