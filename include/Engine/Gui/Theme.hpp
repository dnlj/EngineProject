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

			struct {
				float32 pad1;

				/** The cross axis size of scroll bars */
				float32 scrollWidth = 16;
			} sizes;

			struct Colors {
				glm::vec4 foreground;

				glm::vec4 background; // Main background
				glm::vec4 background2;
				glm::vec4 background3;
				glm::vec4 backgroundAlt; // Alternate background (alternating table rows for example)

				glm::vec4 title;

				glm::vec4 accent;
				glm::vec4 feature;

				glm::vec4 button;


			} colors;
	};
}
