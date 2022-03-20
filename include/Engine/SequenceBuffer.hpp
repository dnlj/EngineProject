#pragma once

// Engine
#include <Engine/Engine.hpp>


namespace Engine {
	template<class I> // TODO: move
	ENGINE_INLINE constexpr bool seqGreater(I a, decltype(a) b) noexcept {
		constexpr auto half = std::numeric_limits<I>::max() / 2 + 1;
		return ((a > b) && (a - b <= half))
			|| ((a < b) && (b - a >  half));
	}

	template<class I> // TODO: move
	ENGINE_INLINE constexpr bool seqLess(I a, decltype(a) b) noexcept {
		return seqGreater<I>(b, a);
	}

	// TODO: doc
	template<class S, class T, S N> // TODO: Doc destructive to old entities as new are inserted
	class SequenceBuffer { // Almost SparseSet + RingBuffer. Not quite.
		private:
			struct Entry {
				S seq;
				bool valid = false;
			};

			S lowest = 0;
			S next = 0;
			Entry entries[N] = {};
			T storage[N];

			constexpr static S index(S seq) noexcept { return seq % N; }

			ENGINE_INLINE auto& getEntry(S seq) { return entries[index(seq)]; }
			ENGINE_INLINE const auto& getEntry(S seq) const { return const_cast<SequenceBuffer*>(this)->getEntry(seq); }

		public:
			constexpr static S capacity() noexcept { return N; }

			ENGINE_INLINE S max() const noexcept { return next - 1; }
			ENGINE_INLINE S min() const noexcept { return next - capacity(); }
			ENGINE_INLINE S minValid() const noexcept { return lowest; }
			ENGINE_INLINE S span() const noexcept { return max() - minValid() + 1; }

			// TODO: max - N vs lowest

			ENGINE_INLINE void clear() {
				memset(&entries, 0, sizeof(entries));
			}

			ENGINE_INLINE bool entryAt(S seq) const {
				return getEntry(seq).valid;
			}

			ENGINE_INLINE bool canInsert(S seq) const {
				return seqGreater(seq, min()) || seq == min();
			}
			
			T& insertNoInit(S seq) {
				ENGINE_DEBUG_ASSERT(canInsert(seq), "Attempting to insert invalid entry.");

				if (const S n = seq + 1; seqGreater(n, next)) {
					S s = next;
					next = n;
					while (seqLess(s, n)) {
						remove(s);
						++s;
					}
				}

				// TODO: what if there are zero entries? lowest == max() but lowest isnt valid
				if (seqLess(seq, lowest)) {
					lowest = seq;
				}

				getEntry(seq) = {
					.seq = seq,
					.valid = true,
				};

				auto& data = get(seq);
				return data;
			}
			
			T& insert(S seq) {
				return insertNoInit(seq) = T();
			}

			// TODO: doc that data is not removed until overwritten
			ENGINE_INLINE void remove(S seq) {
				getEntry(seq).valid = false;

				while (seqLess(lowest, next) && !getEntry(lowest).valid) {
					++lowest;
				}
			}

			ENGINE_INLINE bool contains(S seq) const {
				const auto& e = entries[index(seq)];
				return (e.valid && (e.seq == seq));
			}

			ENGINE_INLINE T* find(S seq) {
				return contains(seq) ? &get(seq) : nullptr;
			}

			ENGINE_INLINE const T* find(S seq) const {
				return const_cast<SequenceBuffer*>(this)->find(seq);
			}

			ENGINE_INLINE T& get(S seq) {
				return storage[index(seq)];
			}

			ENGINE_INLINE const T& get(S seq) const {
				return const_cast<SequenceBuffer*>(this)->get(seq);
			}
	};
}
