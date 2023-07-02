#pragma once

// Engine
#include <Engine/UI/TextBox.hpp>


namespace Engine::UI {
	class SuggestionHandler {
		public:
			virtual void prepair(Panel* relative) = 0;
			virtual void filter(std::string_view text) = 0;
			virtual void done() = 0;
	};

	class InputTextBox : public TextBox {
		private:
			SuggestionHandler* suggest = nullptr;

		public:
			using TextBox::TextBox;

			virtual bool onAction(ActionEvent action) override;
			virtual void onBeginFocus() override;
			virtual void onEndFocus() override;
			virtual void onTextCallback(std::string_view text) override;

			void setHandler(SuggestionHandler* handler) noexcept {
				ENGINE_DEBUG_ASSERT(suggest == nullptr);
				suggest = handler;
			}
	};
}
