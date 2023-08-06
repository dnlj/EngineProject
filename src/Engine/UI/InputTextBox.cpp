// Engine
#include <Engine/UI/DirectionalLayout.hpp>
#include <Engine/UI/InputTextBox.hpp>


namespace Engine::UI {
	bool InputTextBox::onAction(ActionEvent action) {
		auto result = suggest->onAction(action);

		if (auto text = suggest->get(); !text.empty()) {
			const std::string_view curr = getText();
			if (curr.substr(0, curr.find(' ')) != text) {
				setText(std::string{text} + ' ');
			}
		}

		if (result) { return result; }

		result = TextBox::onAction(action);
		if (result) {
			switch (action) {
				case Action::Cut:
				case Action::Paste:
				case Action::DeletePrev:
				case Action::DeleteNext: {
						ENGINE_DEBUG_ASSERT(suggest);
						suggest->filter(this, getText());
				}
			}
		}
		return result;
	}

	void InputTextBox::onBeginFocus() {
		TextBox::onBeginFocus();
	}

	void InputTextBox::onEndFocus() {
		TextBox::onEndFocus();
		suggest->close();
	}

	void InputTextBox::onTextCallback(std::string_view text) {
		TextBox::onTextCallback(text);

		ENGINE_DEBUG_ASSERT(suggest);
		const auto& full = getText();
		if (!full.empty()) {
			suggest->filter(this, full);
		}
	}
}
