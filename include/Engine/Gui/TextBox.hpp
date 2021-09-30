#pragma once

// Engine
#include <Engine/Gui/Label.hpp>
#include <Engine/Unicode/UTF8.hpp>


namespace Engine::Gui {
	class TextBox : public StringLine {
		private:
			uint32 caretCluster = 0;
			uint32 caretIndex = 0;
			float32 caretX = 0;
			glm::vec2 pad = {5,5}; // TODO: probably pull from font size / theme

		public:
			using StringLine::StringLine;

			ENGINE_INLINE void autoSize() {
				StringLine::autoSize();
				StringLine::offset(pad);
				setSize(getSize() + pad + pad);
			}

			virtual void render(Context& ctx) const override {
				glm::vec2 pos = {0,0};
				const glm::vec2 size = getSize();
				const glm::vec4 bg = {0.3,0.3,0.3,1};
				const glm::vec4 bo = {0,0,0,1};

				ctx.drawRect(pos, size, bg);

				ctx.drawRect(pos, {size.x, 1}, bo);
				ctx.drawRect(pos, {1, size.y}, bo);
				ctx.drawRect(pos + glm::vec2{0, size.y - 1}, {size.x, 1}, bo);
				ctx.drawRect(pos + glm::vec2{size.x - 1, 0}, {1, size.y}, bo);

				pos += pad;

				ctx.drawString(getStringOffset(), &getShapedString());

				if (ctx.getFocus() == this && ctx.isBlinking()) {
					const auto& str = getShapedString();
					//const auto ssize = str.getBounds().getSize();

					ctx.drawRect(
						//pos + glm::vec2{ssize.x, 0},
						pos + glm::vec2{caretX, 0},
						{1, str.getFont()->getLineHeight()},
						bo
					);
				}
			}

			virtual void onAction(Action act) override {
				switch (act) {
					case Action::MoveCharLeft: { moveCharLeft(); break; }
					case Action::MoveCharRight: { moveCharRight(); break; }
					case Action::DeletePrev: { deletePrev(); break; }
					case Action::DeleteNext: { deleteNext(); break; }
					case Action::MoveLineStart: { caretCluster = 0; updateCaretPos(); break; }
					case Action::MoveLineEnd: { caretCluster = -1; updateCaretPos(); deleteNext(); break; }
					case Action::MoveWordLeft: { moveWordLeft(); break; }
					case Action::MoveWordRight: { moveWordRight(); break; }
				}
			}

			virtual void onBeginFocus() override {
				// TODO: use caret pos once correct IME position is fixed (04kVYW2Y)
				ctx->setIMEPosition(getPos());

				ctx->registerTextCallback(this, [this](std::string_view view) { 
					insertText(caretIndex, view);

					// This isnt correct since we use HarfBuzz clusters for
					// moving the caret and this uses Unicode code points.
					// But it gets us there in most cases and doesnt break anything.
					for (byte b : view) {
						caretCluster += !(b & 0b1000'0000) || ((b & 0b1100'0000) == 0b1100'0000);
					}

					updateCaretPos();
					return true;
				});
			};

			virtual void onEndFocus() override {
				ctx->deregisterTextCallback(this);
			};

		private:
			bool isWordSeparator(byte* begin, byte* end) {
				// TODO: also ., -, etc. Does unicode have a class for these things?
				return Unicode::UTF8::isWhitespace(begin, end);
			}

			void moveCharLeft() {
				if (caretCluster > 0) {
					--caretCluster;
					updateCaretPos();
				}
			}

			void moveCharRight() {
				++caretCluster;
				updateCaretPos();
			}

			void moveWordLeft() {
				byte* const begin = reinterpret_cast<byte*>(getTextMutable().data());
				byte* const end = begin + getTextMutable().size();

				while (caretIndex > 0) {
					moveCharLeft();
					byte* curr = begin + caretIndex;
					if (isWordSeparator(Unicode::UTF8::prev(curr, begin), end)) {
						break;
					}
				}
			}

			void moveWordRight() {
				const auto size = static_cast<uint32>(getText().size());
				byte* const begin = reinterpret_cast<byte*>(getTextMutable().data());
				byte* const end = begin + size;

				while (caretIndex < size) {
					byte* curr = begin + caretIndex;
					if (isWordSeparator(curr, end)) {
						moveCharRight();
						break;
					}
					moveCharRight();
				}
			}

			void deleteRangeByClusterIndex(const uint32 begin, const uint32 end) {
				ENGINE_DEBUG_ASSERT(begin <= end);
				const auto& glyphs = getShapedString().getGlyphShapeData();
				uint32 i = 0;
				uint32 last = 0;

				uint32 endByte = 0xFFFFFFFF;
				uint32 beginByte = endByte;

				for (const auto& glyph : glyphs) {
					if (glyph.cluster != last) {
						++i;
						last = glyph.cluster;
					}

					if (i == begin) {
						beginByte = glyph.cluster;
					} else if (i == end) {
						endByte = glyph.cluster;
						break;
					}
				}

				if (beginByte < endByte) {
					getTextMutable().erase(beginByte, endByte - beginByte);
					shape();
				}
			}

			void deletePrev() {
				if (caretCluster > 0) {
					deleteRangeByClusterIndex(caretCluster - 1, caretCluster);
					moveCharLeft();
				}
			}
			
			void deleteNext() {
				deleteRangeByClusterIndex(caretCluster, caretCluster + 1);
			}

			void updateCaretPos() {
				const auto& glyphs = getShapedString().getGlyphShapeData();
				caretX = 0;
				uint32 last = 0;
				uint32 i = 0;
				caretIndex = static_cast<uint32>(getText().size());

				for (const auto& glyph : glyphs) {
					if (glyph.cluster != last) {
						++i;
						last = glyph.cluster;
					}

					if (i == caretCluster) {
						caretIndex = glyph.cluster;
						break;
					}

					caretX += glyph.advance.x;
				}

				if (i < caretCluster) {
					caretCluster = i + 1;
				} else {
					ctx->updateBlinkTime();
				}
			}
	};
}
