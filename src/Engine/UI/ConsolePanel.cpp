// Engine
#include <Engine/UI/Button.hpp>
#include <Engine/UI/ConsolePanel.hpp>
#include <Engine/UI/Context.hpp>
#include <Engine/UI/DirectionalLayout.hpp>
#include <Engine/UI/TextBox.hpp>
#include <Engine/UI/TextFeed.hpp>


namespace Engine::UI {
	ConsolePanel::ConsolePanel(Context* context) : PanelT{context} {
		const auto& theme = ctx->getTheme();

		feed = ctx->createPanel<TextFeed>(this);
		feed->pushText("This is the first line.\rThis is the second line.\nThis is the third line.");
		feed->pushText("This is the fourth line.");
		feed->pushText("");
		feed->pushText("This is the fifth line.\nThis is the sixth line.");

		ctx->addPanelUpdateFunc(feed, [](Panel* self){
			constexpr static auto lcg = [](auto x){ return x * 6364136223846793005l + 1442695040888963407l; };
			static auto last = Clock::now();
			static int i = 0;
			static uint64 rng = [](auto& i){
				auto seed = 0b1010011010000101001101011000011010011110010100110100110100101000ull;
				for (; i < 10'000; ++i) { seed = lcg(seed); }
				return seed;
			}(i);
			auto area = static_cast<TextFeed*>(self);
			const auto now = Clock::now();
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

		input = ctx->constructPanel<TextBox>();
		input->autoText("This is a test abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
		input->setAction([this](TextBox*){ submitInput(); });

		auto submit = ctx->constructPanel<Button>();
		submit->autoText("Submit");
		submit->lockSize();
		submit->setAction([this](Button*){ submitInput(); });

		auto cont = ctx->createPanel<PanelT>(this);
		cont->addChildren({input, submit});
		cont->setLayout(new DirectionalLayout{Direction::Horizontal, Align::Stretch, Align::Start, theme.sizes.pad1});
		cont->autoHeight();
		cont->lockHeight();

		setLayout(new DirectionalLayout{Direction::Vertical, Align::Stretch, Align::Stretch, theme.sizes.pad1});
	}
	
	bool ConsolePanel::onAction(ActionEvent act) {
		switch (act) {
			case Action::MoveCharUp: {
				historyDec();
				return true;
			}
			case Action::MoveCharDown: {
				historyInc();
				return true;
			}
			case Action::Paste:
			default: { return input->onAction(act); }
		}
	}

	void ConsolePanel::submitInput() {
		ctx->setFocus(input);
		const auto& text = input->getText();
		if (text.size() <= 0) { return; }

		historyReset();
		push("> " + text);
		if (text != history.head(-1)) {
			// Make sure the most recent history is always a blank string.
			history.head() = text;
			history.advance();
			history.head().clear();
		}

		if (onSubmitInput) {
			onSubmitInput(this, text);
		}

		input->setText("");
	}

	void ConsolePanel::push(std::string_view text) {
		feed->pushText(text);
	}

	ENGINE_INLINE void ConsolePanel::historyInc() {
		if (historyOff == 0) { return; }
		++historyOff;
		historyUpdate();
	}

	ENGINE_INLINE void ConsolePanel::historyDec() {
		if (historyOff - 1 == history.size()) { return; }
		if (history.head(historyOff-1).empty()) { return; }
		--historyOff;
		historyUpdate();
	}

	ENGINE_INLINE void ConsolePanel::historyReset() {
		historyOff = 0;
	}

	ENGINE_INLINE void ConsolePanel::historyUpdate() {
		// TODO: current index = head/0
		input->setText(history.head(historyOff));
	}
}
