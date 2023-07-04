// Engine
#include <Engine/UI/DirectionalLayout.hpp>
#include <Engine/UI/InputTextBox.hpp>


namespace Engine::UI {
	bool InputTextBox::onAction(ActionEvent action) {
		auto result = suggest->onAction(action);
		if (result) { return result; }

		switch (action) {
			case Action::Submit: {
				if (auto text = suggest->get(); !text.empty()) {
					setText(text);
					suggest->close();
					return true;
				}
			}
		}

		result = TextBox::onAction(action);
		switch (action) {
			case Action::Cut:
			case Action::Paste:
			case Action::DeletePrev:
			case Action::DeleteNext: {
				if (result) {
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
