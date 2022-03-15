#pragma once

// Engine
#include <Engine/Gui/FontManager.hpp>


namespace Engine::Gui {
	class Theme {
		public:
			Theme() = default;
			Theme(Theme&) = delete;

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
				/** Main foreground */
				glm::vec4 foreground;
				glm::vec4 foregroundAlt;

				/** Main background. Numbered varients for distinguishing between nested areas. */
				glm::vec4 background; 
				glm::vec4 background2;
				glm::vec4 background3;

				/** Alternate background (alternating table rows for example) */
				glm::vec4 backgroundAlt;

				glm::vec4 title;

				/** Generic accent color. Notable/interactable element. */
				glm::vec4 accent;

				/** An interactable feature such as the background for a slider or scrollbar */
				glm::vec4 feature;

				/** Button */
				glm::vec4 button; 


			} colors;
	};
}
