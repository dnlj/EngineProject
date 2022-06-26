#pragma once

// Engine
#include <Engine/Gui/CollapsibleSection.hpp>

// Game
#include <Game/UI/ui.hpp>


namespace Game::UI {
	class AutoList : public EUI::CollapsibleSection {
		private:
			std::vector<EUI::Label*> labels;
			std::vector<std::string> formats;
			std::string buffer;

		public:
			AutoList(EUI::Context* context) : CollapsibleSection{context} {
				auto* content = getContent();
				content->setLayout(new EUI::DirectionalLayout{EUI::Direction::Vertical, EUI::Align::Start, EUI::Align::Start, ctx->getTheme().sizes.pad1});

				setAutoSizeHeight(true);
				content->setAutoSizeHeight(true);
			}

			int32 addLabel(const std::string& format) {
				auto* panel = ctx->createPanel<EUI::Label>(getContent());
				auto& label = labels.emplace_back(panel);
				formats.push_back(format);
				label->autoText(format);
				return static_cast<int32>(labels.size()) - 1;
			}

			template<class... Ts>
			void setLabel(int32 id, const Ts&... values) {
				auto panel = labels[id];
				fmt::format_to(std::back_inserter(buffer), formats[id], values...);
				panel->autoText(buffer);
				buffer.clear();
			}
	};
}
