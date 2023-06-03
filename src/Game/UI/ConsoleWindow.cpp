// Engine
#include <Engine/Unicode/UTF8.hpp>
#include <Engine/UI/TextBox.hpp>
#include <Engine/UI/DirectionalLayout.hpp>

// Game
#include <Game/UI/ConsoleWindow.hpp>


namespace Game::UI {
	ConsolePanel::ConsolePanel(EUI::Context* context) : PanelT{context} {
		const auto& theme = ctx->getTheme();

		feed = ctx->createPanel<EUI::TextFeed>(this);
		feed->pushText("This is the first line.\rThis is the second line.\nThis is the third line.");
		feed->pushText("This is the fourth line.");
		feed->pushText("");
		feed->pushText("This is the fifth line.\nThis is the sixth line.");

		ctx->addPanelUpdateFunc(feed, [](Panel* self){
			auto* engine = self->getContext()->getUserdata<EngineInstance>();
			constexpr static auto lcg = [](auto x){ return x * 6364136223846793005l + 1442695040888963407l; };
			static auto last = engine->getWorld().getTime();
			static int i = 0;
			static uint64 rng = [](auto& i){
				auto seed = 0b1010011010000101001101011000011010011110010100110100110100101000ull;
				for (; i < 10'000; ++i) { seed = lcg(seed); }
				return seed;
			}(i);
			auto area = static_cast<EUI::TextFeed*>(self);
			const auto now = engine->getWorld().getTime();
			if (now - last > std::chrono::milliseconds{0}) {
				rng = lcg(rng);
				//area->pushText("This is line " + std::to_string(++i) + " " + std::string(1 + rng%32, 'A') + '!' + '\n');
				area->pushText("This is line " + std::to_string(++i) + " " + std::string(1 + rng%32, 'A') + '!');
				last = now;
			}

			if (i == 10'115) {
				//area->pushText("");
				//area->pushText("abc123");
				//area->pushText("\n");
				//area->pushText("xyz789");
				//area->pushText("foo bar baz\n");
				self->getContext()->clearPanelUpdateFuncs(self);
			}
		});

		input = ctx->constructPanel<EUI::TextBox>();
		input->autoText("This is a test");

		auto submit = ctx->constructPanel<EUI::Button>();
		submit->autoText("Submit");
		submit->lockSize();
		submit->setAction([this](EUI::Button* self){
			ctx->setFocus(input);
			const auto txt = input->getText();
			if (txt.size() <= 0) { return; }
			feed->pushText(txt);
			input->setText("");
		});

		auto cont = ctx->createPanel<EUI::PanelT>(this);
		cont->addChildren({input, submit});
		cont->setLayout(new EUI::DirectionalLayout{EUI::Direction::Horizontal, EUI::Align::Stretch, EUI::Align::Start, theme.sizes.pad1});
		cont->autoHeight();
		cont->lockHeight();

		setLayout(new EUI::DirectionalLayout{EUI::Direction::Vertical, EUI::Align::Stretch, EUI::Align::Stretch, theme.sizes.pad1});
	}
	
	bool ConsolePanel::onAction(EUI::ActionEvent act) {
		using EUI::Action;
		switch (act) {
			//case Action::Scroll: { break; } // TODO: forward to TextFeed
			case Action::Paste:
			default: { return input->onAction(act); }
		}
	}
}

namespace Game::UI {
	ConsoleWindow::ConsoleWindow(EUI::Context* context) : Window{context} {
		setTitle("Console");
		setSize({650, 500});
		setRelPos({512,64+300+8});
		setContent(ctx->constructPanel<ConsolePanel>());
	}
}
