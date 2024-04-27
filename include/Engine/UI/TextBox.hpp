#pragma once

// Engine
#include <Engine/UI/Bindable.hpp>
#include <Engine/UI/Caret.hpp>
#include <Engine/UI/Label.hpp>
#include <Engine/Unicode/UTF8.hpp>


namespace Engine::UI {
	class TextBox : public StringLine, public Bindable<TextBox> {
		public:
			using OnSubmit = std::function<void (TextBox* self)>;

		private:
			uint8 selectingCount = 0;
			Caret caret;
			Caret select;
			OnSubmit onSubmit;

		public:
			TextBox(Context* context) : StringLine{context} {
				setPadding(ctx->getTheme().sizes.pad1);
			}

			ENGINE_INLINE_REL void setText(std::string_view text) {
				StringLine::setText(text);
				moveLineEnd();
				select = Caret::invalid;
			}

			ENGINE_INLINE_REL void autoText(std::string_view text) {
				StringLine::autoText(text);
				moveLineEnd();
				select = Caret::invalid;
			}

			ENGINE_INLINE_REL void insertText(uint32 i, std::string_view text) {
				StringLine::insertText(i, text);
				if (i <= caret.index) {
					caret.index += static_cast<uint32>(text.size());
					updateCaretPos();
				}
			}

			ENGINE_INLINE void setAction(OnSubmit func) { onSubmit = std::move(func); }

			virtual void render() override;
			virtual bool onAction(ActionEvent act) override;
			virtual void onBeginHover() override;
			virtual void onEndHover() override;
			virtual void onBeginFocus() override;
			virtual void onEndFocus() override;
			virtual bool onBeginActivate() override;
			virtual void onEndActivate() override;

			virtual void onTextCallback(std::string_view text, Input::KeyCode code);

		private:
			void tryBeginSelection() noexcept;
			Caret getCaretInLine(const float32 x) const noexcept;
			ENGINE_INLINE bool selecting() const noexcept { return selectingCount > 0; }

			void actionSelectWord();
			void actionSelectAll();
			void actionCancel();
			void actionCut();
			void actionCopy();
			void actionDelete();
			void actionPaste();			
			void actionDeletePrev();			
			void actionDeleteNext();

			void moveCharLeft();
			void moveCharRight();
			void moveWordLeft();
			void moveWordRight();
			void moveWordEndRight();
			void moveLineStart();
			void moveLineEnd();
			void deleteRangeByIndex(const uint32 begin, const uint32 end);
			void updateCaretPos();

			bool isWordSeparator(Unicode::Unit8* begin) const noexcept {
				// TODO: should probably use Unicode Character Categories for these:
				//       https://www.compart.com/en/unicode/category
				//       http://www.unicode.org/reports/tr44/#General_Category_Values
				for (auto c : {'.','-','_',':','/','\\','#'}) {
					if (c == +*begin) { return true; }
				}
				return Unicode::UTF8::isWhitespace(begin);
			}

			ENGINE_INLINE bool shouldMoveCaret() const noexcept {
				// Don't move the caret on the first move after a selection. If
				// you have a selection and end the selection by pressing
				// left/right it should jump to the start/end of the selection
				// and not the char before/after the selection.
				return selecting() || select == Caret::invalid || select == caret;
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
	};
}
