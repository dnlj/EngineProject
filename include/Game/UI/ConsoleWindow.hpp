#pragma once

// Engine
#include <Engine/AlignedStorage.hpp>
#include <Engine/UI/FontGlyphSet.hpp>
#include <Engine/UI/TextSelection.hpp>
#include <Engine/UI/Window.hpp>

// Game
#include <Game/UI/ui.hpp>


namespace Game::UI {
	template<class T>
	class SimpleRingBuffer {
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
			ENGINE_INLINE const T* unsafe_data() const noexcept { return storage.data(); }
			ENGINE_INLINE constexpr static Index capacity() noexcept { return 2048; } // 1 << 13
			ENGINE_INLINE Index getHead() const noexcept { return head; }
			ENGINE_INLINE constexpr T& operator[](Index i) { return storage[i]; }
			ENGINE_INLINE constexpr const T& operator[](Index i) const { return storage[i]; }
			ENGINE_INLINE constexpr static Index wrap(Index i) noexcept { return i & (capacity() - 1); }
			ENGINE_INLINE constexpr static Index subwrap(Index i) noexcept { return wrap(capacity() + i); }
	};

	class TextFeed : public EUI::Panel {
		private:
			using Index = uint32;
			constexpr static Index invalidIndex = -1;

			struct Range {
				Index start;
				Index stop;
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
				EUI::Bounds bounds; // TODO: rm - dont actually need
			};

			struct Selection { // TODO: can we do a generic TextSelection
				EUI::Caret first;
				EUI::Caret second;
			};

			SimpleRingBuffer<char> charBuff;

			// TODO: should really use RingBuffer for this, but we dont have an insert(first,last) funciton for that yet.
			Engine::RingBuffer<EUI::ShapeGlyph> glyphBuff;
			Index glyphIndex = 0;
			Engine::RingBuffer<Line> lines;
			EUI::Font font;
			Selection sel = {};

		public:
			TextFeed(EUI::Context* context);
			void pushText(std::string_view txt);

			void render() override;
			bool onBeginActivate() override;
			void onEndActivate() override;

			bool onAction(EUI::ActionEvent act) override;

		private:
			Index getMaxVisibleLines() const;
			EUI::Caret getCaret();
			int wrap(int i) { return 0; }
			void actionCopy();
	};

	class ConsoleWindow : public EUI::Window {
		private:
			TextFeed* feed;

		public:
			ConsoleWindow(EUI::Context* context);
	};
}
