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

		static int count = 0; // TODO: rm
		const auto pushLine = [this](auto a, auto b) ENGINE_INLINE {
			auto& line = lines.emplace();
			line.chars.start = static_cast<Index>(charBuff.getHead());
			if (a == b) { return; }

			// TODO: this is wrong, we dont wrap index, should be solved when we switch to ring buffer.
			charBuff.push(a, b);
			line.chars.stop = static_cast<Index>(charBuff.getHead());
			ENGINE_DEBUG_ASSERT(charBuff.getHead() < std::numeric_limits<Index>::max(), "Char index is to large to fit in datatype.");

			EUI::Bounds bounds;
			line.glyphs.start = static_cast<Index>(glyphBuff.size());
			font->shapeString({a, b}, glyphBuff, bounds);
			line.glyphs.stop = static_cast<Index>(glyphBuff.size());
			ENGINE_DEBUG_ASSERT(glyphBuff.size() < std::numeric_limits<Index>::max(), "Glyph index is to large to fit in datatype.");
			ENGINE_LOG("Add ", ++count, " ", std::string_view{a,b}, " [", line.chars.start, ", ", line.chars.stop, ")");

			{ // TODO: we actually should be able to do this after we have pushed all lines? or is there potential problem with overlapping our whole buffer with a single push of multiple lines? idk. ill think about this later. probably just needs extra checks.
				// Remove old lines that overlap our new buffer
				while (lines.size() > 1) {
					// Any line that overlaps our new line
					const auto& f = lines.front().chars;
					const auto& l = line.chars;

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
					const bool wrapped = l.stop < l.start;
					const bool after = f.start >= l.start;
					const bool before = f.start < l.stop;
					//   (1||2) && (   2    ||   1  )) || (        3       )
					if ((before && (wrapped || after)) || (wrapped && after)) {
						ENGINE_WARN("Pop: [", lines.front().chars.start, ", ", lines.front().chars.stop, ")");
						lines.pop();
						// TODO: glyph cleanup
					} else {
						break;
					}
				}

				ENGINE_WARN("Frt: [", lines.front().chars.start, ", ", lines.front().chars.stop, ")");
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

		const auto base = glyphBuff.data();
		const auto lh = font->getLineHeight();
		float32 yOff = 25;
		const auto xOff = 25;
		for (const auto& line : lines) {
			ctx->drawString({xOff, yOff}, {0,1,0,1}, font, {base + line.glyphs.start, base + line.glyphs.stop});
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
			static int i = 0;
			static auto last = Engine::Clock::now();
			auto area = static_cast<TextArea*>(self);
			const auto now = Engine::Clock::now();
			if (now - last > std::chrono::milliseconds{400}) {
				area->pushText("This is line " + std::to_string(++i));
				last = now;
			}
		});
	}
}
