#pragma once


namespace Engine::Gui {
	class Panel;

	class Layout {
		public:
			virtual ~Layout() {}

			// TODO: should be pure
			virtual float32 getAutoHeight(const Panel* panel) const { return 32; };

			virtual void layout(Panel* panel) = 0;
	};
}
