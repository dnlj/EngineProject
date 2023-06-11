#pragma once

// Engine
#include <Engine/UI/Panel.hpp>


namespace Engine::UI {
	class TextFeed;
	class TextBox;
	class Button;
}

namespace Engine::UI {
	class ConsolePanel : public PanelT {
		public:
			using OnSubmit = std::function<void (ConsolePanel& self, std::string_view text)>;

		protected:
			TextFeed* feed;
			TextBox* input;
			Button* submit;
			OnSubmit onSubmit;

		public:
			ConsolePanel(Context* context);
			void setAction(OnSubmit func) { onSubmit = func; }
			virtual bool onAction(ActionEvent act) override;

			// TODO: up/down command history
	};
}
