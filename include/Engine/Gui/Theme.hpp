#pragma once

// Engine
#include <Engine/Gui/FontManager.hpp>


namespace Engine::Gui {
	class Theme {
		public:
			/* Storage for fonts. @See FontList */
			struct Fonts {
				Font header;
				Font body;
			} fonts;

			struct Colors {
				glm::vec3 foreground;
				glm::vec3 background;
				glm::vec3 accent;
			} colors;
	};
}
