// Engine
#include <Engine/UI/TextBox.hpp>


namespace Engine::UI {
	void TextBox::render() {
		ctx->setClip(getBounds());
		auto& theme = ctx->getTheme();
		const auto& str = getShapedString();
		const auto& font = str.getFont();
		const auto lineHeight = font->getLineHeight();
		const glm::vec2 size = getSize();
		const glm::vec4 bg = {0.3,0.3,0.3,1};
		const glm::vec4 bo = {0,0,0,1};

		ctx->setColor(bg);
		ctx->drawRect({}, size);

		const auto off = getStringOffset();
		const auto topCaret = (getHeight() - lineHeight) * 0.5f; // Center in line

		if (select.valid() && caret != select) {
			const auto a = caret.pos < select.pos ? caret : select;
			const auto b = caret.pos < select.pos ? select : caret;

			ctx->setColor(bo);
			ctx->drawRect(topCaret + glm::vec2{a.pos, 0}, {b.pos - a.pos, lineHeight});

			// Separate color for selection
			if constexpr (true) {
				ctx->setColor(theme.colors.foreground);
				ctx->drawString(off, &str);
			} else {
				const auto& glyphs = str.getGlyphShapeData();
				const auto data = glyphs.data();

				if (a.index != 0) {
					ctx->setColor({1,0,0,1});
					ctx->drawString(off, font, {data, data + a.index});
				}

				ctx->setColor({0,1,0,1});
				ctx->drawString({off.x+a.pos, off.y}, font, {data + a.index, data + b.index});

				if (auto sz = glyphs.size(); b.index != sz) {
					ctx->setColor({0,0,1,1});
					ctx->drawString({off.x+b.pos, off.y}, font, {data + b.index, data + sz});
				}
			}
		} else {
			ctx->setColor(theme.colors.foreground);
			ctx->drawString(off, &str);
		}

		if (ctx->getFocus() == this && ctx->isBlinking()) {
			ctx->setColor(bo);
			ctx->drawRect(
				topCaret + glm::vec2{caret.pos, 0},
				{1, lineHeight}
			);
		}
		
		ctx->setColor(bo);
		ctx->drawRect({}, {size.x, 1});
		ctx->drawRect({}, {1, size.y});
		ctx->drawRect(glm::vec2{0, size.y - 1}, {size.x, 1});
		ctx->drawRect(glm::vec2{size.x - 1, 0}, {1, size.y});
	}

	bool TextBox::onAction(ActionEvent act) {
		// All actions that involve selection are wrapped in a selectingCount
		// inc/dec because selecting() is only true when selectingCount > 1. This is
		// to prevent selection when something changes our caret other than a user
		// action (calling setText when the caret = size for example).
		switch (act) {
			case Action::SelectBegin: { ++selectingCount; break; }
			case Action::SelectEnd: { if (selectingCount > 0) { --selectingCount; }; break; }
			case Action::SelectAll: { ++selectingCount; actionSelectAll(); --selectingCount; break; }
			case Action::MoveCharLeft: { ++selectingCount; moveCharLeft(); --selectingCount; break; }
			case Action::MoveCharRight: { ++selectingCount; moveCharRight(); --selectingCount; break; }
			case Action::MoveLineStart: { ++selectingCount; moveLineStart(); --selectingCount; break; }
			case Action::MoveLineEnd: { ++selectingCount; moveLineEnd(); --selectingCount; break; }
			case Action::MoveWordLeft: { ++selectingCount; moveWordLeft(); --selectingCount; break; }
			case Action::MoveWordRight: { ++selectingCount; moveWordRight(); --selectingCount; break; }
			case Action::DeletePrev: { actionDeletePrev(); break; }
			case Action::DeleteNext: { actionDeleteNext(); break; }
			case Action::Cut: { actionCut(); break; }
			case Action::Copy: { actionCopy(); break; }
			case Action::Paste: { actionPaste(); break; }
			case Action::Submit: { if (onSubmit) { onSubmit(this); } break; }
			default: { return false; }
		}
		return true;
	}

	ENGINE_INLINE void TextBox::tryBeginSelection() noexcept {
		if (selecting() && select == Caret::invalid) {
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

		ctx->registerTextCallback(this, [this](std::string_view text) {
			if (select.valid()) { actionDelete(); }
			onTextCallback(text);
			return true;
		});
	};

	void TextBox::onEndFocus() {
		ctx->deregisterTextCallback(this);
	};
	
	bool TextBox::onBeginActivate() {
		if (ctx->getActive() == this) { return true; }

		caret = getCaretInLine(ctx->getCursor().x);
		select = Caret::invalid;

		ctx->registerMouseMove(this, [this](const glm::vec2 pos) {
			select = getCaretInLine(pos.x);

			// Selections should contain at least one full char
			if (select == caret) { select = Caret::invalid; }
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

	void TextBox::onTextCallback(std::string_view text) {
		insertText(caret.index, text);
		setBindableValue();
	}

	Caret TextBox::getCaretInLine(const float32 x) const noexcept {
		return UI::getCaretInLine(x - getPos().x, getShapedString().getGlyphShapeData());
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
		select = {0,0};
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
		updateCaretPos();
		setBindableValue();
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
		// Make sure our caret is in a valid position before trying to update
		// our selection. This can happen by using setText instead of typing.
		if (auto sz = static_cast<decltype(caret.index)>(getText().size()); caret.index > sz) {
			caret.index = sz;
			updateCaretPos();
		}

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
			setBindableValue();
		}
	}

	void TextBox::updateCaretPos() {
		select = selecting() ? select : Caret::invalid;
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
