#pragma once

// Engine
#include <Engine/UI/TextBox.hpp>


namespace Engine::UI {
	class SuggestionHandler {
		public:
			virtual bool onAction(ActionEvent action) = 0;
			virtual void filter(Panel* relative, std::string_view text) = 0;
			virtual void close() = 0;
			virtual std::string_view get() = 0;
	};

	class InputTextBox : public TextBox {
		private:
			SuggestionHandler* suggest = nullptr;
			Input::KeyCode filterKey = Input::KeyCode::None;

		public:
			using TextBox::TextBox;

			virtual bool onAction(ActionEvent action) override;
			virtual void onBeginFocus() override;
			virtual void onEndFocus() override;
			virtual void onTextCallback(std::string_view text, Input::KeyCode code) override;

			void setFilterKey(Input::KeyCode code) { filterKey = code; }

			void setHandler(SuggestionHandler* handler) noexcept {
				ENGINE_DEBUG_ASSERT(suggest == nullptr);
				suggest = handler;
			}
	};
}
