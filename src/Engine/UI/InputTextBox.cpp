// Engine
#include <Engine/UI/DirectionalLayout.hpp>
#include <Engine/UI/InputTextBox.hpp>


namespace Engine::UI {
	bool InputTextBox::onAction(ActionEvent action) {
		auto result = TextBox::onAction(action);
		switch (action) {
			case Action::Cut:
			case Action::Paste:
			case Action::DeletePrev:
			case Action::DeleteNext: {
				if (result) {
					ENGINE_DEBUG_ASSERT(suggest);
					suggest->filter(getText());
				}
			}
		}
		return result;
	}

	void InputTextBox::onBeginFocus() {
		TextBox::onBeginFocus();

		// TODO: probably want this to be some kind of scroll area
		ENGINE_DEBUG_ASSERT(suggest);
		suggest->prepair(this);
	}

	void InputTextBox::onEndFocus() {
		TextBox::onEndFocus();

		ENGINE_DEBUG_ASSERT(suggest);
		suggest->done();
	}

	void InputTextBox::onTextCallback(std::string_view text) {
		TextBox::onTextCallback(text);

		ENGINE_DEBUG_ASSERT(suggest);
		const auto& full = getText();
		if (!full.empty()) {
			suggest->filter(full);
		}
	}
}
