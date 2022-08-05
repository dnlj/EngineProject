#pragma once

// Engine
#include <Engine/UI/Window.hpp>

namespace Engine::UI {
	class DemoWindow : public Window {
		private:
			Gfx::Texture2D tex;

		public:
			DemoWindow(class Context* context);
	};
};
