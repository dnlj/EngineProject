#pragma once

// Engine
#include <Engine/UI/Panel.hpp>


namespace Engine::UI {
	class TextFeed;
	class TextBox;
	class Button;
}

namespace Engine {
	// TODO: sort out our various ring buffer class
	template<class T, int32 N>
	class DumbRingBuffer {
		public:
			using Index = int32;
			static_assert(N > 0, "The number of elements must be positive.");
			static_assert(std::has_single_bit(static_cast<uint32>(N)), "N must be a power of two.");

		private:
			T storage[N];
			Index nextIndex = {};

		public:
			ENGINE_INLINE void advance() noexcept { nextIndex = wrap(nextIndex+1); }
			ENGINE_INLINE Index next() const noexcept { return nextIndex; }

			template<class U>
			T& push(U&& other) {
				auto& temp = storage[nextIndex];
				temp = std::forward<U>(other);
				advance();
				return temp;
			}

			ENGINE_INLINE T& head(Index offset = 0) noexcept { return storage[wrap(nextIndex - 1 + offset)]; }
			ENGINE_INLINE const T& head(Index offset = 0) const noexcept { return const_cast<DumbRingBuffer&>(*this)[offset]; }
			
			ENGINE_INLINE T& tail(Index offset = 0) noexcept { return storage[wrap(nextIndex + offset)]; }
			ENGINE_INLINE const T& tail(Index offset = 0) const noexcept { return const_cast<DumbRingBuffer&>(*this)[offset]; }

			ENGINE_INLINE constexpr static Index capacity() noexcept { return N; }
			ENGINE_INLINE constexpr static Index size() noexcept { return capacity(); }
			ENGINE_INLINE constexpr static Index wrap(Index i) noexcept { return i & (capacity()-1); }
	};
}

namespace Engine::UI {
	class ConsolePanel final : public PanelT {
		public:
			using OnSubmit = std::function<void (ConsolePanel* self, std::string_view text)>;

		protected:
			TextFeed* feed;
			TextBox* input;
			OnSubmit onSubmit;
			DumbRingBuffer<std::string, 64> history;
			int32 historyOff = {};

		public:
			ConsolePanel(Context* context);
			void setAction(OnSubmit func) { onSubmit = std::move(func); }
			virtual bool onAction(ActionEvent act) override;

			// TODO: up/down command history

		private:
			void doSubmit();
			void historyInc();
			void historyDec();
			void historyReset();
			void historyUpdate();
	};
}
