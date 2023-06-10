#pragma once

// Engine
#include <Engine/UI/Panel.hpp>
#include <Engine/UI/FontGlyphSet.hpp>


namespace Engine::UI {
	template<class T>
	class SimpleRingBuffer { // TODO: move to own file
		private:
			static_assert(std::is_trivially_copyable_v<T>);

			using Index = uint32;
			std::vector<T> storage; // TODO: why use a vector instead of array?
			Index head = 0;

		public:
			SimpleRingBuffer() {
				storage.resize(capacity());
			}

			void push(const T* first, const T* last) {
				Index len1 = static_cast<Index>(last - first);
				Index len2 = 0;
				const auto dat = storage.data();

				if (head + len1 >= capacity()) {
					len2 = wrap(head + len1);
					len1 = len1 - len2;
				}

				memcpy(dat + head, first, len1 * sizeof(T));
				head = wrap(head + len1);

				ENGINE_DEBUG_ASSERT(len2==0 || head==0);
				memcpy(dat + head, first + len1, len2 * sizeof(T));
				head = wrap(head + len2);
			}

			// TODO: why unsafe_? For this buffer everything is initialized already. It might be out of bounds but its not undefined behavior or anything.
			ENGINE_INLINE const T* unsafe_data() const noexcept { return storage.data(); }
			ENGINE_INLINE constexpr static Index capacity() noexcept { return 2048; } // 1 << 13
			ENGINE_INLINE constexpr Index getHead() const noexcept { return head; }
			ENGINE_INLINE constexpr T& operator[](Index i) { return storage[i]; }
			ENGINE_INLINE constexpr const T& operator[](Index i) const { return storage[i]; }
			ENGINE_INLINE constexpr static Index wrap(Index i) noexcept { return i & (capacity() - 1); }
			ENGINE_INLINE constexpr static Index subwrap(Index i) noexcept { return wrap(capacity() + i); }
	};

	class TextFeed : public Panel {
		private:
			using Index = uint32;
			constexpr static Index invalidIndex = -1;

			struct Range {
				Index start = invalidIndex;
				Index stop = invalidIndex;
				ENGINE_INLINE constexpr bool valid() const noexcept { return start != invalidIndex && stop != invalidIndex; }
				[[nodiscard]] ENGINE_INLINE constexpr bool contains(Index i) const noexcept {
					if (stop < start) {
						return i >= start || i < stop;
					} else {
						return i >= start && i < stop;
					}
				};
			};

			struct Line {
				Range chars;
				Range glyphs;
				Bounds bounds;
			};

			struct Caret {
				Index line = invalidIndex;
				Index index = invalidIndex;
				float32 pos = 0;
				ENGINE_INLINE constexpr bool valid() const noexcept { return index != invalidIndex; }
				constexpr bool operator==(const Caret&) const noexcept = default;
			};

			struct Selection { // TODO: can we do a generic TextSelection
				Caret first;
				Caret second;
			};

			SimpleRingBuffer<char> charBuff;

			Engine::RingBuffer<ShapeGlyph> glyphBuff;
			Index glyphIndex = 0;
			Engine::RingBuffer<Line> lines;
			Font font;
			Selection sel = {};
			int32 lineScrollOffset = 0;
			Context::PanelUpdateFuncId autoscrollTimer = {};

		public:
			TextFeed(Context* context);
			void pushText(std::string_view txt);

			void render() override;
			bool onBeginActivate() override;
			void onEndActivate() override;

			bool onAction(ActionEvent act) override;

		private:
			void scroll(float32 numLines);
			void scroll(int32 numLines);
			Selection sortedSelection() const;
			Index getMaxVisibleLines() const;
			Caret getCaret();
			void selectWord();
			void selectLine();
			void selectAll();
			void actionCopy();
	};
}
