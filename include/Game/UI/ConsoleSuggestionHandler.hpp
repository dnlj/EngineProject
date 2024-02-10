#pragma once

// Engine
#include <Engine/UI/InputTextBox.hpp>

// Game
#include <Game/UI/ui.hpp>


// Debug console command suggestions
#if 0
	#define GAME_DEBUG_CONSOLE_SUGGESTIONS(arg) arg
#else
	#define GAME_DEBUG_CONSOLE_SUGGESTIONS(arg)
#endif


namespace Game {
	class CommandManager;
}

namespace Game::UI {
	class ConsoleSuggestionPopup;

	class ConsoleSuggestionLabel final : public EUI::StringLine {
		public:
			using StringLine::StringLine;

			virtual bool onBeginActivate() override;
			virtual void render() override;

		private:
			ConsoleSuggestionPopup* popup();
	};

	class ConsoleSuggestionPopup final : public EUI::Panel {
		private:
			struct Match {
				using Score = int;

				constexpr bool operator>(const Match& right) const noexcept { return score() > right.score(); }
				constexpr bool operator==(const Match& right) const noexcept = delete;
				constexpr Score score() const noexcept { return good + bad; }

				// We could also use a combined score instead of keeping track of
				// good/bad separate and it would be 99% the same. The only downside is
				// we couldn't do simple things like `good <= 0` to cull all results
				// that have zero matches. We could probably use a heuristic or just add
				// a found var if we really wanted to.
				Score good = 0;
				Score bad = 0;

				uint32 index = 0;
				GAME_DEBUG_CONSOLE_SUGGESTIONS(std::string_view full = {});
				GAME_DEBUG_CONSOLE_SUGGESTIONS(std::string highlight = {});
			};

			EUI::InputTextBox* input = nullptr;
			ConsoleSuggestionLabel* selected = nullptr;
			std::vector<Match> matches;

		public:
			ConsoleSuggestionPopup(EUI::Context* context, EUI::InputTextBox* input);
			bool onAction(EUI::ActionEvent action);
			void filter(std::string_view text);
			void clear();
			void select(ConsoleSuggestionLabel* child);
			std::string_view get() const noexcept;
			ENGINE_INLINE EUI::Panel* getSelected() const noexcept { return selected; };
			ENGINE_INLINE EUI::InputTextBox* getInput() const noexcept { return input; }

		private:
			template<auto FirstChild, auto NextChild>
			void select();

			void update(std::string_view text);
	};
	class ConsoleSuggestionHandler final : public EUI::SuggestionHandler {
		private:
			ConsoleSuggestionPopup* popup = nullptr;

		public:
			ConsoleSuggestionHandler(EUI::InputTextBox* input);
			virtual bool onAction(EUI::ActionEvent action) override;
			virtual void filter(EUI::Panel* relative, std::string_view text) override;
			virtual void close() override;
			virtual std::string_view get() override;
	};
}
