#pragma once

// Engine
#include <Engine/Gui/Window.hpp>

namespace Engine::Gui {
	class DemoWindow : public Window {
		private:
			Texture2D tex;

		public:
			DemoWindow(class Context* context);
	};
};
