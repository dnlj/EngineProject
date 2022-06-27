#pragma once


namespace Engine::UI {
	class Panel;

	class Layout {
		public:
			virtual ~Layout() {}

			// TODO: should be pure
			virtual float32 getAutoHeight(const Panel* panel) const { return 53; };
			virtual float32 getAutoWidth(const Panel* panel) const { return 53; };

			virtual void layout(Panel* panel) = 0;
	};
}
