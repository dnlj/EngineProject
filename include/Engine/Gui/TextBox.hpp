#pragma once

// Engine
#include <Engine/Gui/Label.hpp>
#include <Engine/Gui/Bindable.hpp>
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

	class TextBox : public StringLine, public Bindable<TextBox> {
		private:
			uint8 selecting = 0;
			Caret caret = 0;
			Caret select = 0;

		public:
			TextBox(Context* context) : StringLine{context} {
				setPadding(ctx->getTheme().sizes.pad1);
			}

			virtual void render() override;
			virtual bool onAction(ActionEvent act) override;
			virtual void onBeginHover() override;
			virtual void onEndHover() override;
			virtual void onBeginFocus() override;
			virtual void onEndFocus() override;
			virtual bool onBeginActivate() override;
			virtual void onEndActivate() override;

		private:
			void tryBeginSelection() noexcept;

			Caret caretFromPos(const float32 pos) const noexcept;

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
				// TODO: should probably use Unicode Character Categories for these
				// TODO: cont. https://www.compart.com/en/unicode/category
				// TODO: cont. http://www.unicode.org/reports/tr44/#General_Category_Values
				for (auto c : {'.','-','_',':','/','\\','#'}) {
					if (c == +*begin) { return true; }
				}
				return Unicode::UTF8::isWhitespace(begin);
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
	};
}
