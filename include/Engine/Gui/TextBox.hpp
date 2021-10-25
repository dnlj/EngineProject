#pragma once

// Engine
#include <Engine/Gui/Label.hpp>
#include <Engine/Unicode/UTF8.hpp>


namespace Engine::Gui {
	class Caret {
		public:
			constexpr static uint32 invalid = 0xFFFFFFFF;

			uint32 index = 0;
			float32 pos = 0;

			ENGINE_INLINE Caret(const uint32 index = 0, const float32 pos = 0) noexcept : index{index}, pos{pos} {}
			ENGINE_INLINE bool valid() const noexcept { return index != invalid; }
			ENGINE_INLINE friend bool operator==(const Caret& a, const Caret& b) noexcept { return a.index == b.index; }
	};

	class TextBox : public StringLine {
		private:
			uint8 selecting = 0;
			Caret caret = 0;
			Caret select = 0;

			glm::vec2 pad = {5,5}; // TODO: probably pull from font size / theme

		public:
			using StringLine::StringLine;

			ENGINE_INLINE void autoSize() {
				StringLine::autoSize();
				StringLine::offset(pad);
				setSize(getSize() + pad + pad);
			}

			virtual void render() const override {
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

				pos += pad;

				ctx->drawString(getStringOffset(), &str);

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

			virtual void onAction(Action act) override {
				switch (act) {
					case Action::SelectBegin: { ++selecting; if (selecting == 1) { select = caret; } break; }
					case Action::SelectEnd: { if (selecting > 0) { --selecting; }; break; }
					case Action::SelectAll: { actionSelectAll(); break; }
					case Action::MoveCharLeft: { moveCharLeft(); break; }
					case Action::MoveCharRight: { moveCharRight(); break; }
					case Action::DeletePrev: { actionDeletePrev(); break; }
					case Action::DeleteNext: { actionDeleteNext(); break; }
					case Action::MoveLineStart: { caret.index = 0; updateCaretPos(); break; }
					case Action::MoveLineEnd: { caret.index = static_cast<uint32>(getText().size()); updateCaretPos(); break; }
					case Action::MoveWordLeft: { moveWordLeft(); break; }
					case Action::MoveWordRight: { moveWordRight(); break; }
					case Action::Cut: { actionCut(); break; }
					case Action::Copy: { actionCopy(); break; }
					case Action::Paste: { actionPaste(); break; }
				}
			}
			
			virtual void onBeginHover() override {
				ctx->setCursor(Cursor::Text);
			}

			virtual void onEndHover() override {
				ctx->setCursor(Cursor::Normal);
			}

			virtual void onBeginFocus() override {
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

			virtual void onEndFocus() override {
				ctx->deregisterTextCallback(this);
			};
			
			virtual void onBeginActivate() override {
				if (ctx->getActive() == this) { return; }

				caret = caretFromPos(ctx->getCursor().x);
				select = Caret::invalid;

				ctx->registerMouseMove(this, [this](const glm::vec2 pos) {
					select = caretFromPos(pos.x);
				});


				/*
				ENGINE_LOG("Active! ", ctx->getActivateCount());
				const auto count = ctx->getActivateCount() % 2;
				if (count == 0) {
					// TODO: Select word
					ENGINE_LOG("Select word");
					actionSelectWord();
				} else if (count  == 1) {
					// TODO: Select line
					ENGINE_LOG("Select line");
					actionSelectAll();
				}*/
			}

			virtual void onEndActivate() override {
				ctx->deregisterMouseMove(this);
			}

		private:
			Caret caretFromPos(const float32 pos) const noexcept {
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
					if (glyph == glyphs.end()) { ++result.index; break; }
					result.index = glyph->cluster;

					// The multipler for advance used here is just a guess based on observation and
					// feel. A value around 0.6 feels about right. 0.5 is to small. Im not sure how other text
					// engines handle selection. Probably worth looking into to get 100% native feel.
					if (x + result.pos + glyph->advance.x * 0.6f > pos) { break; }
					result.pos += glyph->advance.x;
				}

				return result;
			}

			bool isWordSeparator(Unicode::Unit8* begin) {
				// TODO: should probably use Unicode Character Categories for these
				// TODO: cont. https://www.compart.com/en/unicode/category
				// TODO: cont. http://www.unicode.org/reports/tr44/#General_Category_Values
				for (auto c : {'.','-','_',':','/','\\','#'}) {
					if (c == +*begin) { return true; }
				}
				return Unicode::UTF8::isWhitespace(begin);
			}

			//void actionSelectWord() {
			//	ENGINE_LOG("Select Word222222222");
			//	moveWordLeft();
			//	onAction(Action::SelectBegin);
			//	moveWordRight();
			//	onAction(Action::SelectEnd);
			//}

			void actionSelectAll() {
				onAction(Action::SelectBegin);
				onAction(Action::MoveLineEnd);
				onAction(Action::SelectEnd);
				select = {};
			}

			void actionCancel() {
				select = Caret::invalid;
			}

			void actionCut() {
				if (select == Caret::invalid) { return; }
				actionCopy();
				actionDelete();
				actionCancel();
			}

			void actionCopy() {
				if (select == Caret::invalid) { return; }
				auto base = std::min(caret.index, select.index);
				auto sz = std::max(caret.index, select.index) - base;
				ctx->setClipboard(std::string_view{getText().data() + base, sz});
			}

			ENGINE_INLINE void actionDelete() {
				deleteRangeByIndex(std::min(caret.index, select.index), std::max(caret.index, select.index));
			}

			void actionPaste() {
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
			
			void actionDeletePrev() {
				if (select != Caret::invalid) {
					actionDelete();
				} else if (caret.index > 0) {
					deleteRangeByIndex(getIndexOfPrevCodePoint(), caret.index);
				}
			}
			
			void actionDeleteNext() {
				if (select.index != Caret::invalid) {
					actionDelete();
				} else {
					deleteRangeByIndex(caret.index, getIndexOfNextCodePoint());
				}
			}

			ENGINE_INLINE bool shouldMoveCaret() const noexcept {
				return selecting || select == Caret::invalid || select == caret;
			}

			ENGINE_INLINE const Unicode::Unit8* getCodePointAtCaret() const {
				return reinterpret_cast<const Unicode::Unit8*>(getText().data() + caret.index);
			}

			ENGINE_INLINE const Unicode::Unit8* getCodePointAtBegin() const {
				return reinterpret_cast<const Unicode::Unit8*>(getText().data());
			}

			ENGINE_INLINE const Unicode::Unit8* getCodePointAtEnd() const {
				return reinterpret_cast<const Unicode::Unit8*>(getText().data() + getText().size());
			}

			ENGINE_INLINE uint32 codePointToIndex(const Unicode::Unit8* ptr) const {
				return static_cast<uint32>(ptr - getCodePointAtBegin());
			}

			ENGINE_INLINE uint32 getIndexOfNextCodePoint() const noexcept {
				return codePointToIndex(Unicode::next(getCodePointAtCaret(), getCodePointAtEnd()));
			}

			ENGINE_INLINE uint32 getIndexOfPrevCodePoint() const noexcept {
				return codePointToIndex(Unicode::prev(getCodePointAtCaret(), getCodePointAtBegin()));
			}

			void moveCharLeft() {
				if (shouldMoveCaret()) {
					caret.index = getIndexOfPrevCodePoint();
				} else {
					caret.index = std::min(caret.index, select.index);
				}
				updateCaretPos();
			}

			void moveCharRight() {
				if (shouldMoveCaret()) {
					caret.index = getIndexOfNextCodePoint();
				} else {
					caret.index = std::max(caret.index, select.index);
				}
				updateCaretPos();
			}

			void moveWordLeft() {
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

			void moveWordRight() {
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

			void deleteRangeByIndex(const uint32 begin, const uint32 end) {
				if (begin < end) {
					getTextMutable().erase(begin, end - begin);
					shape();

					caret.index = begin;
					selecting = 0;
					updateCaretPos();
				}
			}

			void updateCaretPos() {
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
	};
}
