// Engine
#include <Engine/Gui/TextBox.hpp>


namespace Engine::Gui {
	void TextBox::render() {
		auto& theme = ctx->getTheme();
		glm::vec2 pos = {0,0};
		const auto& str = getShapedString();
		const glm::vec2 size = getSize();
		const glm::vec4 bg = {0.3,0.3,0.3,1};
		const glm::vec4 bo = {0,0,0,1};

		ctx->drawRect(pos, size, bg);

		ctx->drawRect(pos, {size.x, 1}, bo);
		ctx->drawRect(pos, {1, size.y}, bo);
		ctx->drawRect(pos + glm::vec2{0, size.y - 1}, {size.x, 1}, bo);
		ctx->drawRect(pos + glm::vec2{size.x - 1, 0}, {1, size.y}, bo);

		pos += getStringOffset();
		ctx->drawString(pos, &str, theme.colors.foreground);
		pos.y -= str.getFont()->getAscent();


		if (ctx->getFocus() == this && ctx->isBlinking()) {
			ctx->drawRect(
				pos + glm::vec2{caret.pos, 0},
				{1, str.getFont()->getLineHeight()},
				bo
			);
		}

		if (select.valid()) {
			const auto a = caret.pos < select.pos ? caret.pos : select.pos;
			const auto b = caret.pos < select.pos ? select.pos : caret.pos;

			ctx->drawRect(
				pos + glm::vec2{a, 0},
				{b - a, str.getFont()->getLineHeight()},
				bo
			);
		}
	}

	bool TextBox::onAction(ActionEvent act) {
		switch (act) {
			case Action::SelectBegin: { ++selecting; break; }
			case Action::SelectEnd: { if (selecting > 0) { --selecting; }; break; }
			case Action::SelectAll: { actionSelectAll(); break; }
			case Action::MoveCharLeft: { moveCharLeft(); break; }
			case Action::MoveCharRight: { moveCharRight(); break; }
			case Action::DeletePrev: { actionDeletePrev(); break; }
			case Action::DeleteNext: { actionDeleteNext(); break; }
			case Action::MoveLineStart: { moveLineStart(); break; }
			case Action::MoveLineEnd: { moveLineEnd(); break; }
			case Action::MoveWordLeft: { moveWordLeft(); break; }
			case Action::MoveWordRight: { moveWordRight(); break; }
			case Action::Cut: { actionCut(); break; }
			case Action::Copy: { actionCopy(); break; }
			case Action::Paste: { actionPaste(); break; }
			case Action::Submit: { ENGINE_WARN("TODO: Gui::TextBox Submit - ", getText()); break; }
			default: { return false; }
		}
		return true;
	}

	ENGINE_INLINE void TextBox::tryBeginSelection() noexcept {
		if (selecting && select == Caret::invalid) {
			select = caret;
		}
	}
			
	void TextBox::onBeginHover() {
		ctx->setCursor(Cursor::Text);
	}

	void TextBox::onEndHover() {
		ctx->setCursor(Cursor::Normal);
	}

	void TextBox::onBeginFocus() {
		// TODO: use caret pos once correct IME position is fixed (04kVYW2Y)
		ctx->setIMEPosition(getPos());

		ctx->registerTextCallback(this, [this](std::string_view view) {
			if (select.valid()) { actionDelete(); }
			insertText(caret.index, view);
			caret.index += static_cast<uint32>(view.size());
			updateCaretPos();
			return true;
		});
	};

	void TextBox::onEndFocus() {
		ctx->deregisterTextCallback(this);
	};
			
	bool TextBox::onBeginActivate() {
		if (ctx->getActive() == this) { return true; }

		caret = caretFromPos(ctx->getCursor().x);
		select = Caret::invalid;

		ctx->registerMouseMove(this, [this](const glm::vec2 pos) {
			select = caretFromPos(pos.x);
		});

		if (auto count = ctx->getActivateCount(); count > 1) {
			count %= 2;
			if (count == 0) {
				actionSelectWord();
			} else if (count == 1) {
				actionSelectAll();
			}
		}

		return true;
	}

	void TextBox::onEndActivate() {
		ctx->deregisterMouseMove(this);
	}

	Caret TextBox::caretFromPos(const float32 pos) const noexcept {
		const auto x = getPos().x;
		const auto& glyphs = getShapedString().getGlyphShapeData();
		Caret result = {};

		// Use glyph advances to approximate glyph bbox.
		// To do this "correctly" we would have to fully calculate the glyph
		// bbox(adv+off+width), which could overlap glyphs leading to strange selections
		// where you end up selecting a code point that occurs visually after but byte
		// order before. Unless a problem arises, using advances is better in my opinion
		// because of more obvious selection.

		for (auto glyph = glyphs.begin();; ++glyph) {
			if (glyph == glyphs.end()) {
				result.index += !glyphs.empty();
				break;
			}
			result.index = glyph->cluster;

			// The multipler for advance used here is just a guess based on observation and
			// feel. A value around 0.6 feels about right. 0.5 is to small. Im not sure how other text
			// engines handle selection. Probably worth looking into to get 100% native feel.
			if (x + result.pos + glyph->advance.x * 0.6f > pos) { break; }
			result.pos += glyph->advance.x;
		}

		return result;
	}

	void TextBox::actionSelectWord() {
		moveWordLeft();
		onAction(Action::SelectBegin);
		moveWordEndRight();
		onAction(Action::SelectEnd);
	}

	void TextBox::actionSelectAll() {
		onAction(Action::SelectBegin);
		onAction(Action::MoveLineEnd);
		onAction(Action::SelectEnd);
		select = {};
	}

	ENGINE_INLINE void TextBox::actionCancel() {
		select = Caret::invalid;
	}

	void TextBox::actionCut() {
		if (select == Caret::invalid) { return; }
		actionCopy();
		actionDelete();
		actionCancel();
	}

	void TextBox::actionCopy() {
		if (select == Caret::invalid) { return; }
		auto base = std::min(caret.index, select.index);
		auto sz = std::max(caret.index, select.index) - base;
		ctx->setClipboard(std::string_view{getText().data() + base, sz});
	}

	ENGINE_INLINE void TextBox::actionDelete() {
		deleteRangeByIndex(std::min(caret.index, select.index), std::max(caret.index, select.index));
	}

	void TextBox::actionPaste() {
		const auto text = ctx->getClipboardText();
		if (text.empty()) { return; }

		// Delete current selection
		if (select != Caret::invalid) {
			actionDelete();
		}

		// Insert
		insertText(caret.index, text);
		caret.index += static_cast<uint32>(text.size());
		updateCaretPos();
	}
			
	void TextBox::actionDeletePrev() {
		if (select != Caret::invalid) {
			actionDelete();
		} else if (caret.index > 0) {
			deleteRangeByIndex(getIndexOfPrevCodePoint(), caret.index);
		}
	}
			
	void TextBox::actionDeleteNext() {
		if (select.index != Caret::invalid) {
			actionDelete();
		} else {
			deleteRangeByIndex(caret.index, getIndexOfNextCodePoint());
		}
	}

	void TextBox::moveCharLeft() {
		tryBeginSelection();
		if (shouldMoveCaret()) {
			caret.index = getIndexOfPrevCodePoint();
		} else {	
			caret.index = std::min(caret.index, select.index);
		}
		updateCaretPos();
	}

	void TextBox::moveCharRight() {
		tryBeginSelection();
		if (shouldMoveCaret()) {
			caret.index = getIndexOfNextCodePoint();
		} else {
			caret.index = std::max(caret.index, select.index);
		}
		updateCaretPos();
	}

	void TextBox::moveWordLeft() {
		tryBeginSelection();
		auto* const begin = reinterpret_cast<Unicode::Unit8*>(getTextMutable().data());

		// Start at the first non-separator
		while (caret.index > 0 && isWordSeparator(Unicode::UTF8::prev(begin + caret.index, begin))) {
			moveCharLeft();
		}

		// Go to start of next word
		while (caret.index > 0 && !isWordSeparator(Unicode::UTF8::prev(begin + caret.index, begin))) {
			moveCharLeft();
		}
	}

	void TextBox::moveWordRight() {
		tryBeginSelection();
		const auto size = static_cast<uint32>(getText().size());
		auto* const begin = reinterpret_cast<Unicode::Unit8*>(getTextMutable().data());

		while (caret.index < size) {
			// Found end of next word
			if (isWordSeparator(begin + caret.index)) {
				// Skip multiple separators
				do { moveCharRight(); } while (isWordSeparator(begin + caret.index));
				break;
			}
			moveCharRight();
		}
	}

	void TextBox::moveWordEndRight() {
		const auto size = static_cast<uint32>(getText().size());
		auto* const begin = reinterpret_cast<Unicode::Unit8*>(getTextMutable().data());

		while (caret.index < size) {
			if (isWordSeparator(begin + caret.index)) { break; }
			moveCharRight();
		}
	}
	
	void TextBox::moveLineStart() {
		tryBeginSelection();
		caret.index = 0;
		updateCaretPos();
	}

	void TextBox::moveLineEnd() {
		tryBeginSelection();
		caret.index = static_cast<uint32>(getText().size());
		updateCaretPos();
	}

	void TextBox::deleteRangeByIndex(const uint32 begin, const uint32 end) {
		if (begin < end) {
			getTextMutable().erase(begin, end - begin);
			shape();

			caret.index = begin;
			updateCaretPos();
		}
	}

	void TextBox::updateCaretPos() {
		select = selecting ? select : Caret::invalid;
		const auto last = caret.pos;
		const auto& glyphs = getShapedString().getGlyphShapeData();
		caret.pos = 0;

		for (const auto& glyph : glyphs) {
			if (glyph.cluster >= caret.index) { break; }
			caret.pos += glyph.advance.x;
		}

		if (caret.pos != last) {
			ctx->updateBlinkTime();
		}
	}
}
