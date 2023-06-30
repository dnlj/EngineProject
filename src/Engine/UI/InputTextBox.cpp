// Engine
#include <Engine/UI/DirectionalLayout.hpp>
#include <Engine/UI/InputTextBox.hpp>

namespace {
	std::vector<std::string> getTestData();
}

namespace Engine::UI {
	class SuggestionPopup : public Panel {
		public:
			InputTextBoxModel model;

			SuggestionPopup(Context* context)
				: Panel{context} {

				model.items = getTestData();
			}

			void filter(std::string_view text) {
				// TODO: would be good to have a way to disable updates while we rebuild this list
				clear();
				model.filter(text);
				if (model.empty()) { return; }

				std::vector<Panel*> children;
				children.reserve(model.size());

				for (const auto& i : model) {
					auto label = ctx->constructPanel<Label>();
					label->autoText(model.at(i));
					children.push_back(label);
				}

				addChildren(children);
			}

			void clear() {
				const auto first = getFirstChildRaw();
				if (first == nullptr) { return; }

				const auto last = getLastChildRaw();
				ctx->deferedDeletePanels(first, last);
			}
	};
}

namespace Engine::UI {
	bool InputTextBox::onAction(ActionEvent action) {
		auto result = TextBox::onAction(action);
		switch (action) {
			case Action::Cut:
			case Action::Paste:
			case Action::DeletePrev:
			case Action::DeleteNext: {
				if (result) {
					ENGINE_DEBUG_ASSERT(popup, "Popup should exist by now in focus stack.");
					popup->filter(getText());
				}
			}
		}
		return result;
	}

	void InputTextBox::onBeginFocus() {
		TextBox::onBeginFocus();

		// TODO: probably want this to be some kind of scroll area
		ENGINE_DEBUG_ASSERT(popup == nullptr);
		popup = ctx->createPanel<SuggestionPopup>(ctx->getRoot());
		popup->setEnabled(false);
		popup->setAutoSize(true);
		popup->setLayout(new DirectionalLayout(Direction::Vertical, Align::Start, Align::Stretch, 0));
		popup->setPos(getPos() + glm::vec2{0, getHeight()});
	}

	void InputTextBox::onEndFocus() {
		TextBox::onEndFocus();

		ENGINE_DEBUG_ASSERT(popup != nullptr);
		ctx->deferedDeletePanel(popup);
		popup = nullptr;
	}

	void InputTextBox::onTextCallback(std::string_view text) {
		TextBox::onTextCallback(text);

		popup->setEnabled(true);
		popup->filter(getText());
	}
}


namespace {
std::vector<std::string> getTestData() {
	std::vector<std::string> data = {};
	return data;
}
}
