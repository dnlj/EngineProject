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

			constexpr static Index capacity() noexcept { return 2048; } // 1 << 13
			Index getHead() const noexcept { return head; }

		private:
			ENGINE_INLINE constexpr static Index wrap(Index i) noexcept {
				return i & (capacity() - 1);
			}
	};

	class TextFeed : public EUI::Panel {
		private:
			using Index = uint32;

			struct Range {
				Index start;
				Index stop;
			};

			struct Line {
				Range chars;
				Range glyphs;
				EUI::Bounds bounds; // TODO: rm - dont actually need
			};

			struct Selection { // TODO: can we do a generic TextSelection
				EUI::Caret first = {};
				EUI::Caret second = {};
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

		private:
			EUI::Caret getCaret();
			int wrap(int i) { return 0; }
	};

	class ConsoleWindow : public EUI::Window {
		private:
			TextFeed* feed;

		public:
			ConsoleWindow(EUI::Context* context);
	};
}
