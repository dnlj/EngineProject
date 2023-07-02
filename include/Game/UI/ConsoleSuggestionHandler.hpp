#pragma once

// Engine
#include <Engine/UI/InputTextBox.hpp>

// Game
#include <Game/UI/ui.hpp>


namespace Game::UI {
	class SuggestionModel {
		public:
			using Index = uint32;

		public: // TODO: private
			std::vector<std::string> items; // TODO: shouldnt store a copy, pull from real model
			std::vector<Index> active;

		public:
			void filter(std::string_view text);

			Index size() const noexcept { return static_cast<Index>(active.size()); };
			bool empty() const noexcept { return active.empty(); }
			const auto& at(Index i) const noexcept { return items[i]; }
			auto begin() const { return active.begin(); }
			auto end() const { return active.end(); }
	};

	class ConsoleSuggestionPopup final : public EUI::Panel {
		private:
			SuggestionModel model;

		public:
			ConsoleSuggestionPopup(EUI::Context* context);
			void filter(std::string_view text);
			void clear();

	};

	class ConsoleSuggestionHandler final : public EUI::SuggestionHandler {
		private:
			ConsoleSuggestionPopup* popup = nullptr;

		public:
			virtual void prepair(EUI::Panel* relative) override;
			virtual void filter(std::string_view text) override;
			virtual void done() override;
	};
}
