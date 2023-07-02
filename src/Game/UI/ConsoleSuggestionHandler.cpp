// Engine
#include <Engine/UI/DirectionalLayout.hpp>

// Game
#include <Game/UI/ConsoleSuggestionHandler.hpp>


namespace {
	std::vector<std::string> getTestData() {
		return {
			#include "../.private/testdata_ue"
		};
	}
}


namespace Game::UI {
	void SuggestionModel::filter(std::string_view text) {
		active.clear();

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

		struct Match {
			std::string_view str;
			std::string temp; // TODO: rm

			constexpr bool operator<(const Match& right) const noexcept { return score() < right.score(); }
			constexpr bool operator>(const Match& right) const noexcept { return score() > right.score(); }
			constexpr bool operator==(const Match& right) const noexcept = delete;

			// TODO: combine, this is just useful for debugging
			int good = 0;
			int bad = 0;
			constexpr int score() const noexcept { return good + bad; }
		};

		// Tried using a fixed std::array. A few ms faster in debug. No definitive diff in release.
		std::vector<Match> results;
		results.resize(10, {{}, {}, 0, INT_MIN});

		for (const auto& item : items) {

			auto icur = item.begin();
			const auto iend = item.end();

			auto tcur = text.begin();
			const auto tend = text.end();

			// Lying about being in a sequence initially gives us a bias toward matches at the exact start.
			int goodRun = 3;
			int badRun = 0;
			Match match = {item};

			while (icur != iend && tcur != tend) {
				if (auto quality = eq(*icur, *tcur)) {
					match.temp.push_back(*icur);
					++tcur;

					badRun = 0;
					++goodRun;

					// Encourage exact matches over approx matches.
					// Give increasing bonuses to long sequences of matches.
					match.good += quality + (goodRun >> 2);
				} else {
					match.temp.push_back('_');

					goodRun = 0;
					++badRun;

					// Discourage sequences of bad matches.
					// 
					// Have a little buffer here instead of a constant value
					// helps us give preference to matches that contain the full
					// filter string.
					match.bad -= (badRun > 2);
				}
				++icur;
			}

			// Bias toward shorter/more exact matches
			{
				const int rem = static_cast<int>(iend - icur);
				match.bad -= (rem > 0) + (rem >> 2);
			}

			match.temp.insert(match.temp.end(), item.size() - match.temp.size(), '_');

			// Insert new entry.
			const auto end = results.end();
			auto found = std::lower_bound(results.begin(), end, match, std::greater<Match>{});
			if (found != end) {
				std::shift_right(found, end, 1);
				*found = std::move(match);
			}
		}

		ENGINE_LOG2("{}", std::string(32, '>'));
		{
			const auto end = results.rend();
			auto cur = end - std::min(results.size(), 100ull);
			for (; cur != end; ++cur) {
				ENGINE_LOG2("Match: {} {}\n   {}\n   {}", cur->good, cur->bad, cur->temp, cur->str);
			}
		}
		ENGINE_LOG2("{}", std::string(32, '<'));
	};
}


namespace Game::UI {
	ConsoleSuggestionPopup::ConsoleSuggestionPopup(EUI::Context* context)
		: Panel{context} {
		model.items = getTestData();
	}

	void ConsoleSuggestionPopup::filter(std::string_view text) {
		clear();
		model.filter(text);
		if (model.empty()) { return; }

		// TODO: would be good to have a way to disable updates while we rebuild this list instead of creating a vector
		std::vector<Panel*> children;
		children.reserve(model.size());

		for (const auto& i : model) {
			auto label = ctx->constructPanel<EUI::Label>();
			label->autoText(model.at(i));
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
		auto start = Engine::Clock::now();
		std::atomic_signal_fence(std::memory_order_acq_rel);
		popup->filter(text);
		std::atomic_signal_fence(std::memory_order_acq_rel);
		auto end = Engine::Clock::now();
		ENGINE_INFO2("Time: {}", Engine::Clock::Milliseconds{end - start});
	}

	void ConsoleSuggestionHandler::done() {
		ENGINE_DEBUG_ASSERT(popup);
		popup->getContext()->deferedDeletePanel(popup);
		popup = nullptr;
	}
}
