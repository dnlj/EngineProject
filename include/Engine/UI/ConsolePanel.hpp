#pragma once

// Engine
#include <Engine/UI/Panel.hpp>


namespace Engine::UI {
	class TextFeed;
	class TextBox;
}

namespace Engine::UI {
	class ConsolePanel : public PanelT {
		protected:
			TextFeed* feed;
			TextBox* input;

		public:
			ConsolePanel(Context* context);
			virtual bool onAction(ActionEvent act) override;

			// TODO: up/down command history
	};
}
