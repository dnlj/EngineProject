#pragma once

// Engine
#include <Engine/UI/TextBox.hpp>


namespace Engine::UI {
	class InputTextBoxModel {
		public:
			using Index = uint32;

		public: // TODO: private
			std::vector<std::string> items;
			std::vector<Index> active;

		public:
			// TODO: look at DataAdapter, might be better to use create/update panel methods like that
			void updated(Index first, Index last) {};

			void filter(std::string_view text) { // TODO: move to cpp
				active.clear();

				// TODO: it would probably be better to just store them in lower case? Then we might need to also store original. Idk.
				constexpr auto eq = [](char a, char b) noexcept ENGINE_INLINE {
					return std::tolower(a) == std::tolower(b);
				};

				// TODO: score is tricky:
				//       - Exact match vs remap match
				//       - Sequential matches
				//       - World starts: CamelCase, under_score, lowerCamel, word spaces,
				//       - Really any match around a break (non alphanum, space, upper, start, end, etc.). See UTF8.hpp
				//       - I think distance from start should have the same effect as negative matched letters? I think so.

				std::string temp;
				ENGINE_LOG2("{}", std::string(32, '>'));
					
				const auto sz = items.size();
				for (Index i = 0; i < sz; ++i) {
					const auto& item = items[i];

					auto icur = item.begin();
					const auto iend = item.end();
					auto tcur = text.begin();
					const auto tend = text.end();

					int matched = 0; // TODO: rm

					while (icur != iend && tcur != tend) {
						if (eq(*icur, *tcur)) {
							++matched;
							temp.push_back(*icur);
							++tcur;
						} else {
							temp.push_back('_');
						}
						++icur;
					}

					if (matched > 4) {
						temp.insert(temp.end(), item.size() - temp.size(), '_');
						ENGINE_LOG2("Match:\n{}\n{}\n{}", temp, text, item);
					}
					temp.clear();
					
					//if (item.find(text) != std::string::npos) {
					//	active.push_back(i);
					//}
				}
				ENGINE_LOG2("{}", std::string(32, '<'));
			};

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
