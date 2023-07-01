#pragma once

// Engine
#include <Engine/UI/TextBox.hpp>


namespace Engine::UI {
	class InputTextBoxModel {
		public:
			using Index = uint32;

		public: // TODO: private
			std::vector<std::string> items; // TODO: shouldnt store a copy, pull from real model
			std::vector<Index> active;

		public:
			// TODO: look at DataAdapter, might be better to use create/update panel methods like that
			void updated(Index first, Index last) {};

			void filter(std::string_view text);

			Index size() const noexcept { return static_cast<Index>(active.size()); };
			bool empty() const noexcept { return active.empty(); }
			const auto& at(Index i) const noexcept { return items[i]; }
			auto begin() const { return active.begin(); }
			auto end() const { return active.end(); }
	};

	class InputTextBox : public TextBox {
		private:
			class SuggestionPopup* popup = nullptr;

		public:
			using TextBox::TextBox;
			virtual bool onAction(ActionEvent action) override;
			virtual void onBeginFocus() override;
			virtual void onEndFocus() override;
			virtual void onTextCallback(std::string_view text) override;
	};
}
