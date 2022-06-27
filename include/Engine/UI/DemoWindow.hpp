#pragma once

// Engine
#include <Engine/Gui/Window.hpp>

namespace Engine::UI {
	class DemoWindow : public Window {
		private:
			Texture2D tex;

		public:
			DemoWindow(class Context* context);
	};
};
