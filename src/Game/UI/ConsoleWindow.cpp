// Engine
#include <Engine/Unicode/UTF8.hpp>
#include <Engine/UI/TextBox.hpp>
#include <Engine/UI/DirectionalLayout.hpp>

// Game
#include <Game/UI/ConsoleWindow.hpp>

namespace Game::UI {
	TextFeed::TextFeed(EUI::Context* context) : Panel{context} {
		font = ctx->getTheme().fonts.body;
	}

	void TextFeed::pushText(std::string_view txt) {
		auto beg = std::to_address(txt.begin());
		const auto end = std::to_address(txt.end());
		auto cur = beg;

		constexpr auto isEOL = [](const auto* p) ENGINE_INLINE {
			// TODO: Likely incompleted. I didnt find a unicode set or annex for these. There probably is one I just dont know what its called.
			return (*p == 0x0000) // NUL, null
				|| (*p == 0x0003) // ETX, end of text
				|| (*p == 0x0004) // EOT, end of transmission
				|| (*p == 0x0019) // EOM, end of medium
				|| (*p == 0x0017) // ETB, end of transmission block
				|| (*p == 0x009C) // ST, string terminator
				|| Engine::Unicode::isNewline8(p);
		};

		const auto pushLine = [this](auto a, auto b) ENGINE_INLINE_REL {
			// Dont overflow our buffer
			if (b - a > charBuff.capacity()) {
				a = b - charBuff.capacity();
			}

			// Setup initial line state, needed for correct cleanup and blank lines.
			auto& line = lines.emplace();
			line.chars.start = static_cast<Index>(charBuff.getHead());
			line.chars.stop = line.chars.start;
			line.glyphs.start = glyphIndex;
			line.glyphs.stop = glyphIndex; // Must be set for correct cleanup, even for empty lines.
			if (a == b) { return; }

			// Add our line to the buffer
			charBuff.push(a, b);
			line.chars.stop = static_cast<Index>(charBuff.getHead());
			ENGINE_DEBUG_ASSERT(charBuff.getHead() < uint64{std::numeric_limits<Index>::max()}, "Char index is to large to fit in datatype.");

			// Shape glyphs
			{ 
				EUI::Bounds _unused_bounds;
				const auto szA = glyphBuff.size();
				font->shapeString({a, b}, glyphBuff, _unused_bounds);
				const auto szB = glyphBuff.size();

				line.glyphs.start = glyphIndex;
				glyphIndex += szB - szA;
				line.glyphs.stop = glyphIndex;
				ENGINE_DEBUG_ASSERT(glyphBuff.size() < uint64{std::numeric_limits<Index>::max()}, "Glyph index is to large to fit in datatype.");
				ENGINE_DEBUG_ASSERT(charBuff.capacity() < uint64{std::numeric_limits<Index>::max()}, "Char buffer larger than glyph index can support. Will need to change increase size of glyph index.");
			}

			// Remove old lines that overlap our new buffer
			while (lines.size() > 1) {
				const auto& f = lines.front();
				const auto& l = line;

				////////////////////////////////////////////////////
				// Visualization of overlap logic.
				// B = begin (start)         E = end (stop)
				// Case A = no index wrap    Case B = index wrap 
				//                             B     E
				// Case A: [-------------------|-----|-]
				//                                ^ Area 1
				//              E                 B
				// Case B: [----|-----------------|----]
				//            ^ Area 2               ^ Area 3
				////////////////////////////////////////////////////
				const bool wrapped = l.chars.stop < l.chars.start;
				const bool after = f.chars.start >= l.chars.start;
				const bool before = f.chars.start < l.chars.stop;
				// ( (1||2) && (   2    ||   1  )) || (        3       )
				if ((before && (wrapped || after)) || (wrapped && after)) {
					glyphBuff.remove(glyphBuff.wrap(f.glyphs.stop));
					lines.pop();
				} else {
					break;
				}
			}
		};

		// Split and append lines
		while (cur != end) {
			// TODO: doesnt handle "CR LF". See `Engine::Unicode::UTF32::isNewline`
			if (isEOL(cur)) {
				pushLine(beg, cur);
				++cur; // Skip the EOL
				if (cur == end) { break; }
				beg = cur;
			}

			++cur;
		}

		pushLine(beg, cur);
	}

	void TextFeed::render() {
		// TODO: line wrap or hscroll

		ctx->drawRect({}, getSize(), {0,0,0,0.5});
		
		const auto lh = font->getLineHeight();
		const int32 maxLines = static_cast<int32>(std::ceil(getHeight() / lh));
		const int32 sz = static_cast<int32>(lines.size());
		const int32 upper = sz - 1;
		const int32 lower = std::max(sz - maxLines, 0);

		float32 yOff = getHeight();
		const auto xOff = 0;
		const auto base = glyphBuff.unsafe_dataT();
		ctx->setClip(getBounds());

		for (int32 i = upper; i >= lower; --i) {
			auto& line = lines[i];
			const auto start = glyphBuff.wrap(line.glyphs.start);
			const auto stop = glyphBuff.wrap(line.glyphs.stop);
			if (stop < start) {
				const auto off = ctx->drawString({xOff, yOff}, {0,1,0,1}, font, {base + start, base + glyphBuff.capacity()});
				ctx->drawString(off, {0,1,0,1}, font, {base, base + stop});
			} else {
				ctx->drawString({xOff, yOff}, {0,1,0,1}, font, {base + start, base + stop});
			}
			yOff -= lh;
		}
	};
}

namespace Game::UI {
	ConsoleWindow::ConsoleWindow(EUI::Context* context) : Window{context} {
		const auto& theme = ctx->getTheme();
		setTitle("Console");
		setSize({650, 500});
		setRelPos({512,64+300+8});

		// TODO: can we just add a setContent(p) function? would make sense, instead of creating an extra content panel with a fill layout?
		//getContent()->setLayout(new EUI::FillLayout{0});
		feed = ctx->createPanel<TextFeed>(getContent());
		feed->setHeight(395);
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
			auto area = static_cast<TextFeed*>(self);
			const auto now = engine->getWorld().getTime();
			if (now - last > std::chrono::milliseconds{00}) {
				rng = lcg(rng);
				area->pushText("This is line " + std::to_string(++i) + " " + std::string(1 + rng%32, 'A'));
				last = now;
			}

			if (i == 10'100) { self->getContext()->clearPanelUpdateFuncs(self); }
		});

		auto input = ctx->constructPanel<EUI::TextBox>();
		input->autoText("This is a test");

		auto submit = ctx->constructPanel<EUI::Button>();
		submit->autoText("Submit");
		submit->lockSize();
		submit->setAction([input, feed=feed](EUI::Button* self){
			const auto txt = input->getText();
			if (txt.size() <= 0) { return; }
			feed->pushText(txt);
			input->setText("");
		});

		auto cont = ctx->createPanel<EUI::PanelT>(getContent());
		cont->addChildren({input, submit});
		cont->setAutoSizeHeight(true);
		cont->setLayout(new EUI::DirectionalLayout{EUI::Direction::Horizontal, EUI::Align::Stretch, EUI::Align::Start, theme.sizes.pad1});
	}
}
