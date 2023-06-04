// Engine
#include <Engine/UI/Caret.hpp>
#include <Engine/UI/Context.hpp>
#include <Engine/UI/TextFeed.hpp>
#include <Engine/Unicode/UTF8.hpp>


namespace Engine::UI {
	TextFeed::TextFeed(Context* context) : Panel{context} {
		font = ctx->getTheme().fonts.body;
	}

	void TextFeed::pushText(std::string_view txt) {
		auto beg = std::to_address(txt.begin());
		const auto end = std::to_address(txt.end());
		auto cur = beg;

		constexpr auto isEOL = [](const char* p) ENGINE_INLINE {
			return (*p == 0x0000) // NUL, null
				|| (*p == 0x0003) // ETX, end of text
				|| (*p == 0x0004) // EOT, end of transmission
				|| (*p == 0x0019) // EOM, end of medium
				|| (*p == 0x0017) // ETB, end of transmission block
				|| (*p == 0x009C) // ST, string terminator
				|| Engine::Unicode::isNewline8(p);
		};

		constexpr auto afterEOL = [](const char* begin, const char* end) noexcept ENGINE_INLINE {
			if (end - begin >= 2 && *begin == char{0x000D} && *(begin+1) == char{0x000A})  {
				return begin + 2;
			}
			return begin + 1;
		};

		const auto pushLine = [this](auto a, auto b){
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
			line.bounds = {{0,0},{0,0}};
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
					const auto clampLine = [&](Caret& caret) ENGINE_INLINE {
						if (caret.valid()) {
							if (caret.line == 0) {
								caret.index = lines[1].chars.start;
								caret.pos = 0;
							} else {
								--caret.line;
							}
						}
					};

					clampLine(sel.first);
					clampLine(sel.second);

					glyphBuff.remove(glyphBuff.wrap(f.glyphs.stop));
					lines.pop();
				} else {
					break;
				}
			}
		};

		// Pushed empty line
		if (beg == end) {
			pushLine(beg, end);
			return;
		}
		
		// Split and append lines
		while (cur != end) {
			if (isEOL(cur)) {
				pushLine(beg, cur);
				cur = afterEOL(cur, end);
				beg = cur;
				if (cur == end) { break; }
			} else {
				++cur;
			}
		}

		ENGINE_DEBUG_ASSERT(cur == end);
		if (beg != end){
			pushLine(beg, cur);
		}
	}

	void TextFeed::render() {
		const Index lineCount = static_cast<Index>(lines.size());
		if (lineCount <= 0) { return; }

		// TODO: line wrap or hscroll
		// TODO: allow scrolling blank above/below the text by up to 75% of height()

		ctx->setColor({0,0,0,0.5});
		ctx->drawRect({}, getSize());

		const auto lh = font->getLineHeight();
		const auto eolWidth = font->getNominalSize().x * 0.6f; // The multiplier is arbitrary. The full width just looks wrong.
		const Index maxLines = getMaxVisibleLines();
		const Index latest = (lineScrollOffset > 0 && lineCount <= static_cast<Index>(lineScrollOffset)) ? 0 : lineCount - 1 - lineScrollOffset;
		const Index oldest = latest < maxLines ? 0 : latest + 1 - maxLines;

		// Draw selection
		[&]() ENGINE_INLINE {
			if (sel.second.valid() && sel.first != sel.second) {
				auto [beg, end] = sortedSelection();

				// Out of view
				if (beg.line > latest || end.line < oldest) { return; }

				// Don't show line above/below the screen.
				// We don't have to update index here because we dont actually use that for selection drawing.
				// Above:
				if (beg.line < oldest) {
					beg.line = oldest;
					beg.pos = 0;
				}

				// Below:
				if (end.line > latest) {
					end.line = latest;
					end.pos = lines[latest].bounds.getWidth();
				}

				// Draw
				const auto last = latest + 1; // Needed for exclusive
				float32 yOff = getHeight() - font->getDescent();
				ctx->setColor({1,0,0,1});
				if (beg.line == end.line) { // Single line
					ctx->drawRect({beg.pos, yOff - (last - end.line)*lh}, {end.pos - beg.pos, lh});
				} else { // Multi line
					ctx->drawRect(
						{beg.pos, yOff - (last - beg.line)*lh},
						{lines[beg.line].bounds.getWidth() - beg.pos + eolWidth, lh}
					);
					for (Index i = beg.line + 1; i < end.line; ++i) {
						ctx->drawRect({0, yOff - (last - i)*lh}, {lines[i].bounds.getWidth() + eolWidth, lh});
					}
					ctx->drawRect({0, yOff - (last - end.line)*lh}, {end.pos, lh});
				}
			}
		}();
		
		{ // Draw Text
			float32 yOff = getHeight();
			const auto base = glyphBuff.unsafe_dataT();

			for (Index i = latest;; --i) {
				const auto& line = lines[i];
				const auto start = glyphBuff.wrap(line.glyphs.start);
				const auto stop = glyphBuff.wrap(line.glyphs.stop);

				ctx->setColor({0,1,0,1});
				if (stop < start) {
					const auto off = ctx->drawString({0, yOff}, font, {base + start, base + glyphBuff.capacity()});
					ctx->setColor({0,1,1,1});
					ctx->drawString(off, font, {base, base + stop});
					ctx->setColor({0,1,0,1});
				} else {
					ctx->drawString({0, yOff}, font, {base + start, base + stop});
				}
				yOff -= lh;

				if (i == oldest) { break; }
			}
		}
	}

	bool TextFeed::onBeginActivate() {
		// TODO: why is this needed? Is it possible to activate multiple times?
		if (ctx->getActive() == this) { return true; }

		sel = {};
		sel.first = getCaret();

		ctx->registerMouseMove(this, [this, dir = 0](const glm::vec2 mpos) mutable {
			sel.second = getCaret();
			
			if (mpos.y <= getPos().y) {
				if (dir <= 0) {
					dir = 1;
					if (autoscrollTimer) { ctx->deleteTimer(autoscrollTimer); }
					autoscrollTimer = ctx->createTimer(ctx->getAutoscrollSpeed(), [&]{
						scroll((getPos().y - ctx->getCursor().y) / font->getLineHeight());
					});
				}
			} else if (mpos.y >= getPos().y + getHeight()) {
				if (dir >= 0) {
					dir = -1;
					if (autoscrollTimer) { ctx->deleteTimer(autoscrollTimer); }
					autoscrollTimer = ctx->createTimer(ctx->getAutoscrollSpeed(), [&]{
						scroll((getPos().y + getHeight() - ctx->getCursor().y) / font->getLineHeight());
					});
				}
			} else if (autoscrollTimer) {
				ctx->deleteTimer(autoscrollTimer);
				autoscrollTimer = {};
				dir = 0;
			}
		});

		if (auto count = ctx->getActivateCount(); count > 1) {
			count %= 3;
			if (count == 0) {
				selectLine();
			} else if (count == 1) {
				selectAll();
			} else if (count == 2) {
				selectWord();
			}
		}

		return true;
	}

	void TextFeed::onEndActivate() {
		if (autoscrollTimer) {
			ctx->deleteTimer(autoscrollTimer);
			autoscrollTimer = {};
		}
		ctx->deregisterMouseMove(this);
	}

	bool TextFeed::onAction(ActionEvent act) {
		switch (act) {
			case Action::SelectAll: { selectAll(); break; }
			case Action::Cut: { actionCopy(); break; }
			case Action::Copy: { actionCopy(); break; }
			case Action::Scroll: {
				scroll(static_cast<int32>(act.value.f32 * ctx->getScrollLines()));
				break;
			}
			default: { return false; }
		}
		return true;
	}

	void TextFeed::scroll(float32 numLines) {
		// TODO: support partial line scrolling
		scroll(static_cast<int32>(numLines));
	}

	void TextFeed::scroll(int32 numLines) {
		//ENGINE_DEBUG_ASSERT(numLines != 0, "Attempting to scroll zero lines. This is probably not intended.");
		lineScrollOffset += numLines;
		// TODO: we dont allow overscroll yet
		const auto sz = static_cast<int32>(lines.size());
		lineScrollOffset = std::clamp(lineScrollOffset, 0, sz);

	}

	void TextFeed::selectWord() {
		const auto isWordBreak = [](const void* c){ return Unicode::isWhitespace8(c); };
		const auto data = charBuff.unsafe_data();
		const auto& line = lines[sel.first.line];
		auto beg = sel.first.index;

		// Find the previous word boundary
		while (beg != line.chars.start) {
			if (isWordBreak(data + beg)) {
				break;
			}
			beg = charBuff.subwrap(beg - 1);
		}

		// Skip the break itself. We might be at the start of a line and that
		// char might not be a word break.
		if (isWordBreak(data + beg)) {
			beg = charBuff.wrap(beg + 1);
		}
		
		// Find the next word boundary
		auto end = beg;
		while (end != line.chars.stop) {
			if (isWordBreak(data + end)) {
				break;
			}
			end = charBuff.wrap(end + 1);
		}

		// We don't need to skip the end break because the range is half open: [beg, end)
		sel.first.index = beg;
		sel.second.line = sel.first.line;
		sel.second.index = end;

		{ // Find the render bounds
			const auto glyphs = glyphBuff.unsafe_dataT();
			auto i = line.glyphs.start;
			float32 pos = 0;

			constexpr auto dist = [](auto a, auto b) ENGINE_INLINE {
				return b >= a ? b - a : b + decltype(charBuff)::capacity() - a;
			};
			const auto bdist = dist(line.chars.start, beg);

			while (i != line.glyphs.stop) {
				const auto& glyph = glyphs[glyphBuff.wrap(i)];
				if (glyph.cluster == bdist) { break; }
				pos += glyph.advance.x;
				++i;
			}

			sel.first.pos = pos;

			const auto edist = dist(line.chars.start, end);
			while (i != line.glyphs.stop) {
				const auto& glyph = glyphs[glyphBuff.wrap(i)];
				if (glyph.cluster == edist) { break; }
				pos += glyph.advance.x;
				++i;
			}

			sel.second.pos = pos;
		}
	}
	void TextFeed::selectLine() {
		ENGINE_DEBUG_ASSERT(sel.first.valid());
		const auto& line = lines[sel.first.line];
		sel.first.index = line.chars.start;
		sel.first.pos = 0;
		sel.second.line = sel.first.line;
		sel.second.index = line.chars.stop;
		sel.second.pos = line.bounds.getWidth();
	}

	void TextFeed::selectAll() {
		if (!lines.empty()) {
			sel.first.line = 0;
			sel.first.index = lines.front().chars.start;
			sel.first.pos = 0;

			sel.second.line = lines.size() - 1;
			sel.second.index = lines.back().chars.stop;
			sel.second.pos = lines.back().bounds.getWidth();
		}
	}

	void TextFeed::actionCopy() {
		if (!sel.second.valid()) { return; }
		if (sel.first == sel.second) { return; }

		const auto data = charBuff.unsafe_data();
		const auto [beg, end] = sortedSelection();

		if (beg.line == end.line) {
			if (end.index == beg.index) {
				ENGINE_WARN("Copying empty selection. This should have already been aborted by this point.");
				ENGINE_DEBUG_BREAK;
			}

			if (end.index >= beg.index) {
				ctx->setClipboard({data + beg.index, end.index - beg.index});
			} else {
				ctx->setClipboard({
					{data + beg.index, data + charBuff.capacity()},
					{data, data + end.index}
				});
			}
		} else {
			constexpr std::string_view newline = "\u000A"; // Use '\n' only. Don't use '\r'+'\n' on Windows.
			std::vector<std::string_view> toCopy;
			if (end.line <= beg.line) {
				ENGINE_WARN("Invalid selection lines (end < beg)");
				ENGINE_DEBUG_BREAK;
				return;
			}
			toCopy.reserve(end.line - beg.line);

			constexpr auto capacity = decltype(charBuff)::capacity();
			const auto copy = [&](Range chars) ENGINE_INLINE_REL {
				if (chars.start == chars.stop) { return; }
				if (chars.stop < chars.start) {
					toCopy.emplace_back(data + chars.start, capacity - chars.start);
					toCopy.emplace_back(data, chars.stop);
				} else {
					toCopy.emplace_back(data + chars.start, chars.stop - chars.start);
				}
			};

			copy({beg.index, lines[beg.line].chars.stop});
			toCopy.push_back(newline);

			for (Index i = beg.line+1; i < end.line; ++i) {
				copy(lines[i].chars);
				toCopy.push_back(newline);
			}

			copy({lines[end.line].chars.start, end.index});
			ctx->setClipboard(toCopy);
		}
	}

	auto TextFeed::sortedSelection() const -> Selection {
		auto beg = sel.first.index < sel.second.index ? sel.first : sel.second;
		auto end = sel.first.index < sel.second.index ? sel.second : sel.first;
		
		// Selection wraps?
		const auto head = charBuff.getHead();
		if (beg.index <= head && end.index > head) {
			std::swap(beg, end);
		} else if (end.line < beg.line) {
			// This happens when you select from the end of one line to the
			// start of another (only select the'\n').
			ENGINE_DEBUG_ASSERT(beg.index == end.index);
			std::swap(beg, end);
		}

		ENGINE_DEBUG_ASSERT(beg.line <= end.line);
		return {beg, end};
	}

	auto TextFeed::getMaxVisibleLines() const -> Index {
		return static_cast<Index>(std::ceil(getHeight() / font->getLineHeight()));
	}

	auto TextFeed::getCaret() -> Caret {
		const auto pos = ctx->getCursor();
		const auto rel = pos - getPos();

		const auto lineSz = lines.size();
		const auto lineNum = [&]() -> Index ENGINE_INLINE {
			if (rel.y < 0) { return getMaxVisibleLines(); }
			if (rel.y > getHeight()) { return 0; }

			const auto yOff = getHeight() - rel.y;
			if (yOff < 0) {
				// Shouldn't be possible since we would be out of focus
				ENGINE_DEBUG_ASSERT(false, "Invalid caret line offset");
				return 0;
			} 

			return static_cast<int32>(yOff / font->getLineHeight());
		}() + lineScrollOffset;

		ENGINE_DEBUG_ASSERT(lineScrollOffset >= 0, "TODO: Update getCaret to support negative scroll index"); // TODO: support negative scroll index

		const auto lineOff = 1 + lineNum;
		if (lineOff > lineSz) {
			return {
				.line = 0,
				.index = lines[0].chars.start,
				.pos = 0,
			};
		}

		const auto lineIdx = lineSz - lineOff;
		const auto& line = lines[lineIdx];
		const auto base = glyphBuff.unsafe_dataT();
		const auto start = glyphBuff.wrap(line.glyphs.start);
		const auto stop = glyphBuff.wrap(line.glyphs.stop);
		ArrayView<const ShapeGlyph> view;
		Engine::UI::Caret caret = {0,0};
		
		if (stop < start) {
			view = {base + start, base + glyphBuff.capacity()};
			caret = getCaretInLine(rel.x, view);

			// Hit EoL on first half
			if (caret.index > view.back().cluster) {
				const auto last = caret;
				view = {base, base + stop};
				caret = getCaretInLine(rel.x - last.pos, view);
				caret.pos += last.pos;

				// We don't need to offset index because each line is shaped
				// contiguously before getting pushed to the buffer.
				//caret.index += last.index;
			}
		} else {
			view = {base + start, base + stop};
			caret = getCaretInLine(rel.x, view);
		}

		caret.index = charBuff.wrap(line.chars.start + caret.index);
		return {
			.line = lineIdx,
			.index = caret.index,
			.pos = caret.pos,
		};
	}
}
