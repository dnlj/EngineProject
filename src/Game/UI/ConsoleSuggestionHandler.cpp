// Engine
#include <Engine/UI/DirectionalLayout.hpp>
#include <Engine/CommandManager.hpp>

// Game
#include <Game/UI/ConsoleSuggestionHandler.hpp>


namespace {
	const auto& getCommands(Engine::UI::Context* ctx) {
		auto* instance = ctx->getUserdata<Game::EngineInstance>();
		auto& manager = instance->getCommandManager();
		return manager.getCommands();
	}
}


namespace Game::UI {
	void ConsoleSuggestionPopup::update(std::string_view text) {
		// Scoring is tricky:
		// - Exact match vs remap match
		// - Sequential matches
		// - World starts: CamelCase, under_score, lowerCamel, word spaces,
		// - Really any match around a break (non alphanum, space, upper, start, end, etc.). See UTF8.hpp
		// - I think distance from start should have the same effect as negative matched letters? I think so.
		//
		// -----------------------------------------------------------------------------------------------------------------------
		//        
		// We also have an issue where need to normalize score somehow. Consider:
		// Value1: xxxxxxAB = -1 -1 -1 -1 -1 -1 +1 +1 = -6 +2 = -4
		// Value2: xx       = -1 -1 = -2
		//  Filter: ab
		// 
		// In this situation we score Value1 lower than Value2 even though it contains a full match.
		// On option to consider is to scale negative penalties by value length (this is now basically a percent).
		// That would give: `(-6/8) +1 +1 = 1.25` and `(-2/2) +0 = -1`
		// But we now have the problem that penalties are insignificant relative to bonuses.
		// So we also need to normalize bonuses relative to the FILTER string, but now we are back in the first situation.
		//
		// -----------------------------------------------------------------------------------------------------------------------
		// 
		// Maybe we track bonuses and penalties separate and do something to combine them afterward to generate a global score?
		// I'm not sure what this looks like because if we do a simple sort by bonus then penalty separate we get problems like:
		//
		// Value1: axbyc   = +3 -2
		// Value2: xxxxabc = +3 -4
		// Filter: abc
		//
		// So now Value1 is a better match than Value2. I guess this is why we would need to weigh runs higher.
		// 
		// -----------------------------------------------------------------------------------------------------------------------
		//

		constexpr auto eq = [](char a, char b) noexcept ENGINE_INLINE {
			// 0 = no match, 1 = tolower match, 2 = full match
			return (a == b) + (Engine::toLower(a) == Engine::toLower(b));
		};

		// Tried using a fixed std::array. A few ms faster in debug. No definitive diff in release.
		matches.clear();
		matches.resize(10, { .bad = INT_MIN });

		const auto& commands = getCommands(ctx);
		const auto commandCount = commands.size();
		for (uint32 i = 0; i < commandCount; ++i) {
			auto& item = commands[i].name;

			auto icur = item.begin();
			const auto iend = item.end();

			auto tcur = text.begin();
			const auto tend = text.end();

			// Lying about being in a sequence initially gives us a bias toward matches at the exact start.
			int goodRun = 3;
			int badRun = 0;
			Match match = { .index = i };

			while (icur != iend && tcur != tend) {
				if (auto quality = eq(*icur, *tcur)) {
					GAME_DEBUG_CONSOLE_SUGGESTIONS(match.highlight.push_back(*icur));
					++tcur;

					badRun = 0;
					++goodRun;

					// Encourage exact matches over approx matches.
					// Give increasing bonuses to long sequences of matches.
					match.good += quality + (goodRun >> 2);
				} else {
					GAME_DEBUG_CONSOLE_SUGGESTIONS(match.highlight.push_back('_'));

					goodRun = 0;
					++badRun;

					// Discourage sequences of bad matches.
					// 
					// Having a little buffer here instead of a constant value
					// helps us give preference to matches that contain the full
					// filter string.
					match.bad -= (badRun > 2);
				}
				++icur;
			}

			{ // Bias toward shorter/more exact matches
				const int rem = static_cast<int>(iend - icur);
				match.bad -= (rem > 0) + (rem >> 2);
			}

			GAME_DEBUG_CONSOLE_SUGGESTIONS(
				match.highlight.insert(match.highlight.end(), item.size() - match.highlight.size(), '_');
			);

			{ // Insert new entry
				const auto end = matches.end();
				auto found = std::lower_bound(matches.begin(), end, match, std::greater<Match>{});
				if (found != end) {
					std::shift_right(found, end, 1);
					GAME_DEBUG_CONSOLE_SUGGESTIONS(match.full = item);
					*found = std::move(match);
				}
			}
		}

		{ // Only include positive matches
			const auto end = matches.end();
			auto last = std::find_if(matches.begin(), end, [](const Match& match){ return match.good <= 0; });
			matches.erase(last, end);
		}

		GAME_DEBUG_CONSOLE_SUGGESTIONS({
			ENGINE_LOG2("{}", std::string(32, '>'));
			{
				const auto end = matches.rend();
				auto cur = end - std::min(matches.size(), 100ull);
				for (; cur != end; ++cur) {
					ENGINE_LOG2("Match: {} {}\n   {}\n   {}", cur->good, cur->bad, cur->highlight, cur->full);
				}
			}
			ENGINE_LOG2("{}", std::string(32, '<'));
		})
	};
}


namespace Game::UI {
	ConsoleSuggestionPopup::ConsoleSuggestionPopup(EUI::Context* context)
		: Panel{context} {
	}

	void ConsoleSuggestionPopup::filter(std::string_view text) {
		clear();
		update(text);
		if (matches.empty()) { return; }

		// TODO: would be good to have a way to disable updates while we rebuild this list instead of creating a vector
		std::vector<Panel*> children;
		children.reserve(matches.size());

		auto& commands = getCommands(ctx);
		for (const auto& match : matches) {
			auto label = ctx->constructPanel<EUI::Label>();
			label->autoText(commands[match.index].name);
			children.push_back(label);
		}

		addChildren(children);
	}

	void ConsoleSuggestionPopup::clear() {
		const auto first = getFirstChildRaw();
		if (first == nullptr) { return; }

		const auto last = getLastChildRaw();
		ctx->deferedDeletePanels(first, last);
	}
}

namespace Game::UI {
	void ConsoleSuggestionHandler::prepair(EUI::Panel* relative) {
		using namespace EUI;
		ENGINE_DEBUG_ASSERT(popup == nullptr);
		auto* ctx = relative->getContext();
		popup = ctx->createPanel<ConsoleSuggestionPopup>(ctx->getRoot());
		popup->setEnabled(false);
		popup->setAutoSize(true);
		popup->setLayout(new DirectionalLayout(Direction::Vertical, Align::Start, Align::Stretch, 0));
		popup->setPos(relative->getPos() + glm::vec2{0, relative->getHeight()});
	}

	void ConsoleSuggestionHandler::filter(std::string_view text) {
		ENGINE_DEBUG_ASSERT(popup);
		popup->setEnabled(true);

		GAME_DEBUG_CONSOLE_SUGGESTIONS(
			auto start = Engine::Clock::now();
			std::atomic_signal_fence(std::memory_order_acq_rel);
		)

		popup->filter(text);

		GAME_DEBUG_CONSOLE_SUGGESTIONS(
			std::atomic_signal_fence(std::memory_order_acq_rel);
			auto end = Engine::Clock::now();
			ENGINE_INFO2("Time: {}", Engine::Clock::Milliseconds{end - start});
		)
	}

	void ConsoleSuggestionHandler::done() {
		ENGINE_DEBUG_ASSERT(popup);
		popup->getContext()->deferedDeletePanel(popup);
		popup = nullptr;
	}
}
