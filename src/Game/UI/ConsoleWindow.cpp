// Engine
#include <Engine/Unicode/UTF8.hpp>

// Game
#include <Game/UI/ConsoleWindow.hpp>

namespace Game::UI {
	TextArea::TextArea(EUI::Context* context) : Panel{context} {
		font = ctx->getTheme().fonts.body;
	}

	void TextArea::pushText(std::string_view txt) {
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
			// TOOD: also need to handle the unlikely case where a single line is larger than our charBuff.
			auto& line = lines.emplace();
			line.chars.start = static_cast<Index>(charBuff.getHead());
			line.glyphs.stop = glyphIndex; // Must be set for correct cleanup, even for empty lines.
			if (a == b) { return; }

			charBuff.push(a, b);
			line.chars.stop = static_cast<Index>(charBuff.getHead());
			ENGINE_DEBUG_ASSERT(charBuff.getHead() < uint64{std::numeric_limits<Index>::max()}, "Char index is to large to fit in datatype.");

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

			{ // TODO: we actually should be able to do this after we have pushed all lines? or is there potential problem with overlapping our whole buffer with a single push of multiple lines? idk. ill think about this later. probably just needs extra checks.
				// Remove old lines that overlap our new buffer
				while (lines.size() > 1) {
					// Any line that overlaps our new line
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
					//   (1||2) && (   2    ||   1  )) || (        3       )
					if ((before && (wrapped || after)) || (wrapped && after)) {
						glyphBuff.remove(glyphBuff.wrap(f.glyphs.stop));
						lines.pop();
					} else {
						break;
					}
				}
			}
		};

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

	void TextArea::render() {
		ctx->drawRect({}, getSize(), {0,0,0,0.5});

		const auto base = glyphBuff.unsafe_dataT();
		const auto lh = font->getLineHeight();
		float32 yOff = 25 + (30 - lines.size())*lh;
		const auto xOff = 25;
		for (const auto& line : lines) {
			const auto start = glyphBuff.wrap(line.glyphs.start);
			const auto stop = glyphBuff.wrap(line.glyphs.stop);
			if (stop < start) {
				const auto off = ctx->drawString({xOff, yOff}, {0,1,0,1}, font, {base + start, base + glyphBuff.capacity()});
				ctx->drawString(off, {0,1,0,1}, font, {base, base + stop});
			} else {
				ctx->drawString({xOff, yOff}, {0,1,0,1}, font, {base + start, base + stop});
			}
			yOff += lh;
		}
	};
}

namespace Game::UI {
	ConsoleWindow::ConsoleWindow(EUI::Context* context) : Window{context} {
		setTitle("Console");
		setSize({650, 500});
		setRelPos({512,64+300+8});

		// TODO: can we just add a setContent(p) function? would make sense, instead of creating an extra content panel with a fill layout?
		//getContent()->setLayout(new EUI::FillLayout{0});
		textArea = ctx->createPanel<TextArea>(getContent());
		textArea->setHeight(256);
		textArea->pushText("This is the first line.\rThis is the second line.\nThis is the third line.");
		textArea->pushText("This is the fourth line.");
		textArea->pushText("");
		textArea->pushText("This is the fifth line.\nThis is the sixth line.");
		ctx->addPanelUpdateFunc(textArea, [](Panel* self){
			auto* engine = self->getContext()->getUserdata<EngineInstance>();
			constexpr static auto lcg = [](auto x){ return x * 6364136223846793005l + 1442695040888963407l; };
			static auto last = engine->getWorld().getTime();
			static int i = 0;
			static uint64 rng = [](auto& i){
				auto seed = 0b1010011010000101001101011000011010011110010100110100110100101000ull;
				for (; i < 10'000; ++i) { seed = lcg(seed); }
				return seed;
			}(i);
			auto area = static_cast<TextArea*>(self);
			const auto now = engine->getWorld().getTime();
			if (now - last > std::chrono::milliseconds{200}) {
				rng = lcg(rng);
				area->pushText("This is line " + std::to_string(++i) + " " + std::string(1 + rng%32, 'A'));
				last = now;
			}
		});
	}
}
