#pragma once

// Engine
#include <Engine/UI/InputTextBox.hpp>

// Game
#include <Game/UI/ui.hpp>


// Debug console command suggestions
#if false
	#define GAME_DEBUG_CONSOLE_SUGGESTIONS(arg) arg
#else
	#define GAME_DEBUG_CONSOLE_SUGGESTIONS(arg)
#endif


namespace Game {
	class CommandManager;
}

namespace Game::UI {
	class ConsoleSuggestionPopup final : public EUI::Panel {
		private:
			using Index = uint32;

			struct Match {
				GAME_DEBUG_CONSOLE_SUGGESTIONS(std::string_view full = {});
				GAME_DEBUG_CONSOLE_SUGGESTIONS(std::string highlight = {});

				Index index = 0;

				constexpr bool operator<(const Match& right) const noexcept { return score() < right.score(); }
				constexpr bool operator>(const Match& right) const noexcept { return score() > right.score(); }
				constexpr bool operator==(const Match& right) const noexcept = delete;

				// We could also use a combined score instead of keeping track of
				// good/bad separate and it would be 99% the same. The only downside is
				// we couldn't do simple things like `good <= 0` to cull all results
				// that have zero matches. We could probably use a heuristic or just add
				// a found var if we really wanted to.
				int good = 0;
				int bad = 0;
				constexpr int score() const noexcept { return good + bad; }
			};

			std::vector<Match> matches;
			Panel* selected = nullptr;

		public:
			ConsoleSuggestionPopup(EUI::Context* context);
			bool onAction(EUI::ActionEvent action);
			void filter(std::string_view text);
			void clear();

		private:
			void select();
			void selectNext();
			//void selectPrev();
			void update(std::string_view text);
	};

	class ConsoleSuggestionHandler final : public EUI::SuggestionHandler {
		private:
			ConsoleSuggestionPopup* popup = nullptr;

		public:
			virtual bool onAction(EUI::ActionEvent action) override;
			virtual void prepair(EUI::Panel* relative) override;
			virtual void filter(std::string_view text) override;
			virtual void done() override;
	};
}
