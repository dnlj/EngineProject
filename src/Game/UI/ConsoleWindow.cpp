// Engine
#include <Engine/Unicode/UTF8.hpp>
#include <Engine/UI/TextBox.hpp>
#include <Engine/UI/DirectionalLayout.hpp>

// Game
#include <Game/UI/ConsoleWindow.hpp>

// TODO: move this UI element into engine
using namespace Engine;
using namespace Engine::UI;

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
				const auto szA = glyphBuff.size();
				font->shapeString({a, b}, glyphBuff, line.bounds);
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

		// TODO: if line doesnt end with \n we need to append one

		// Split and append lines
		while (cur != end) {
			// TODO: doesnt handle "CR LF". See `Engine::Unicode::UTF32::isNewline`
			if (isEOL(cur)) {
				++cur; // Skip the EOL
				pushLine(beg, cur);
				beg = cur;
				if (cur == end) { break; }
			} else {
				++cur;
			}
		}

		if (beg != end){
			ENGINE_DEBUG_ASSERT(cur == end);
			pushLine(beg, cur);
		}
	}

	void TextFeed::render() {
		// TODO: line wrap or hscroll
		ctx->setColor({0,0,0,0.5});
		ctx->drawRect({}, getSize());
		
		const auto lh = font->getLineHeight();
		const Index maxLines = getMaxVisibleLines();
		const Index lineCount = static_cast<Index>(lines.size());
		const Index oldest = lineCount < maxLines ? 0 : lineCount - maxLines;
		const Index latest = lineCount - 1;

		// Draw selection
		
		if (sel.second.valid()) {

			// TODO: this fails if we wrap doesnt it?
			auto beg = sel.first.index < sel.second.index ? sel.first : sel.second;
			auto end = sel.first.index < sel.second.index ? sel.second : sel.first;
			Index begLine = invalidIndex;
			Index endLine = invalidIndex;

			const auto head = charBuff.getHead();
			if (beg.index <= head && end.index > head) {
				std::swap(beg, end);
			}

			const auto& TODO_rm123 = lines[latest].chars; TODO_rm123;
			// TODO: could binary search lines here if we need to.
			if (beg.index == lines[latest].chars.stop) {
				// TODO: skip selection drawing. both selections are past end.
				begLine = latest;
				endLine = latest;
			} else {
				for (Index i = 0; i <= latest; ++i) {
					if (lines[i].chars.contains(beg.index)) { begLine = i; break; }
				}
			}
			ENGINE_DEBUG_ASSERT(begLine != invalidIndex);
			
			if (end.index == lines[latest].chars.stop) {
				endLine = latest;
				end.pos = lines[endLine].bounds.getWidth();
			} else {
				for (Index i = begLine; i <= latest; ++i) {
					if (lines[i].chars.contains(end.index)) { endLine = i; break; }
				}
			}
			ENGINE_DEBUG_ASSERT(endLine != invalidIndex);

			// Draw the single or multiline selection
			begLine = std::max(begLine, oldest); // Don't draw offscreen lines
			float32 yOff = getHeight() - font->getDescent();
			ctx->setColor({1,0,0,1});
			if (begLine == endLine) {
				ctx->drawRect({beg.pos, yOff - (lineCount - endLine)*lh}, {end.pos - beg.pos, lh});
			} else {
				// TODO: special case handle eol (maybe just a min width?)
				ctx->drawRect({beg.pos, yOff - (lineCount - begLine)*lh}, {lines[begLine].bounds.getWidth() - beg.pos, lh});
				for (Index i = begLine + 1; i < endLine; ++i) {
					ctx->drawRect({0, yOff - (lineCount - i)*lh}, {lines[i].bounds.getWidth(), lh});
				}
				ctx->drawRect({0, yOff - (lineCount - endLine)*lh}, {end.pos, lh});
			}
		}
		
		{ // Draw Text
			float32 yOff = getHeight();
			const auto base = glyphBuff.unsafe_dataT();

			for (Index i = latest;; --i) {
				auto& line = lines[i];
				const auto start = glyphBuff.wrap(line.glyphs.start);
				const auto stop = glyphBuff.wrap(line.glyphs.stop);

				ctx->setColor({0,1,0,1});
				if (stop < start) {
					const auto off = ctx->drawString({0, yOff}, font, {base + start, base + glyphBuff.capacity()});
					ctx->drawString(off, font, {base, base + stop});
				} else {
					ctx->drawString({0, yOff}, font, {base + start, base + stop});
				}
				yOff -= lh;

				if (i == oldest) { break; }
			}
		}
	}

	// TODO: copy/cut/paste
	bool TextFeed::onAction(EUI::ActionEvent act) {
		using namespace EUI;
		switch (act) {
			// TODO: select all
			// TODO: cut and copy should both just copy
			// TODO: paste should probably be yoinked by parent window
			//case Action::Cut: { actionCut(); break; }
			case Action::Copy: { actionCopy(); break; }
			//case Action::Paste: { actionPaste(); break; }
			default: { return false; }
		}
		return true;
	}
	void TextFeed::actionCopy() {
		ENGINE_LOG("actionCopy 1");
		if (!sel.second.valid()) { return; }
		if (sel.first == sel.second) { return; }
		ENGINE_LOG("actionCopy 2");
		const auto data = charBuff.unsafe_data();
		const auto head = charBuff.getHead();
		const auto a = sel.first.index < sel.second.index ? sel.first.index : sel.second.index;
		const auto b = sel.first.index < sel.second.index ? sel.second.index : sel.first.index;

		if (a < head && b >= head) { // Selection wraps
			std::cout << "^^^^^^^^^^^^^^^\n";
			std::cout << std::string_view{data + b, charBuff.capacity() - b};
			std::cout << std::string_view{data, a};
		} else {
			std::cout << "+++++++++++++++\n";
			std::cout << std::string_view{data + a, b - a};
		}
		std::cout << "\n---------------\n";
	}

	auto TextFeed::getMaxVisibleLines() const -> Index {
		return static_cast<Index>(std::ceil(getHeight() / font->getLineHeight()));
	}

	EUI::Caret TextFeed::getCaret() {
		const auto pos = ctx->getCursor();
		const auto rel = pos - getPos();

		EUI::Caret caret = {0,0};
		const auto lineSz = lines.size();
		const auto lineNum = [&]() ENGINE_INLINE -> Index {
			if (rel.y < 0) { return getMaxVisibleLines(); }
			if (rel.y > getHeight()) { return 0; }
			return static_cast<Index>((getHeight() - rel.y) / font->getLineHeight());
		}();

		const auto& line = lines[lineSz - 1 - lineNum];
		const auto base = glyphBuff.unsafe_dataT();
		const auto start = glyphBuff.wrap(line.glyphs.start);
		const auto stop = glyphBuff.wrap(line.glyphs.stop);
		ArrayView<const ShapeGlyph> view;

		if (stop < start) {
			view = {base + start, glyphBuff.capacity()};
			caret = EUI::getCaretInLine(rel.x, view);

			if (rel.x > caret.pos) {
				const auto last = caret;
				view = {base, stop};
				caret = EUI::getCaretInLine(rel.x - last.pos, view);

				caret.pos += last.pos;
				caret.index += last.index;
			}
		} else {
			view = {base + start, base + stop};
			caret = EUI::getCaretInLine(rel.x, view);
		}

		if (caret.index > view.back().cluster) {
			// Since we wrap we are on start of next line
			caret.pos = 0;
		}

		const auto TODO_rm = caret.index;
		caret.index = charBuff.wrap(line.chars.start + caret.index);
		ENGINE_LOG("Caret: ", caret.index, " ", TODO_rm);
		return caret;

	}

	// TODO: move this into a base calss for both TextBox and TextFeed
	bool TextFeed::onBeginActivate() {
		// TODO: why is this needed? Is it possible to activate multiple times?
		if (ctx->getActive() == this) { return true; }

		sel = {};
		sel.first = getCaret();

		ctx->registerMouseMove(this, [this](const glm::vec2) {
			sel.second = getCaret();
		});

		// TODO:
		//if (auto count = ctx->getActivateCount(); count > 1) {
		//
		//	// TODO: this should rotate betwee: word, paragraph, all
		//
		//	count %= 2;
		//	if (count == 0) {
		//		actionSelectWord();
		//	} else if (count == 1) {
		//		actionSelectAll();
		//	}
		//}

		return true;
	}

	void TextFeed::onEndActivate() {
		// TODO: shift focus to text box at this point
		ctx->deregisterMouseMove(this);
	}
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
				// TODO: append new line causes duplicates
				area->pushText("This is line " + std::to_string(++i) + " " + std::string(1 + rng%32, 'A') + '!' + '\n');
				//area->pushText("This is line " + std::to_string(++i) + " " + std::string(1 + rng%32, 'A') + '!');
				last = now;
			}

			if (i == 10'110) { self->getContext()->clearPanelUpdateFuncs(self); }
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
