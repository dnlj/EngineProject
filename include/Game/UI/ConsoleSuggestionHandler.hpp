#pragma once

// Engine
#include <Engine/UI/InputTextBox.hpp>

// Game
#include <Game/UI/ui.hpp>


namespace Game {
	class CommandManager;
}

namespace Game::UI {
	class ConsoleSuggestionPopup final : public EUI::Panel {
		private:
			using Index = uint32;

			struct Match {
				std::string_view str = {}; // TODO: rm - debugging
				std::string temp = {}; // TODO: rm - debugging
				Index index = 0;

				constexpr bool operator<(const Match& right) const noexcept { return score() < right.score(); }
				constexpr bool operator>(const Match& right) const noexcept { return score() > right.score(); }
				constexpr bool operator==(const Match& right) const noexcept = delete;

				// TODO: combine, this is just useful for debugging
				int good = 0;
				int bad = 0;
				constexpr int score() const noexcept { return good + bad; }
			};

			std::vector<Match> matches;

		public:
			ConsoleSuggestionPopup(EUI::Context* context);
			void filter(std::string_view text);
			void clear();

		private:
			void update(std::string_view text);
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
