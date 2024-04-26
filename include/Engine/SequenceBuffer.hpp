#pragma once

// Engine
#include <Engine/Engine.hpp>


namespace Engine {
	/**
	 * A sparsely populated indexable ring buffer with wrapping indices.
	 * NOTE: When an entry is removed the object stored at that location is not destroyed until it is overwritten.
	 */
	template<class S, class T, S N>
	class SequenceBuffer {
		private:
			S lowest = 0;
			S next = 0;
			bool entries[N] = {};
			T storage[N] = {};

			static_assert(std::is_unsigned_v<S>, "SequenceBuffer assumes a unsigned sequence number.");
			constexpr static S index(S seq) noexcept { return seq % N; }

			ENGINE_INLINE auto& getEntry(S seq) { return entries[index(seq)]; }
			ENGINE_INLINE const auto& getEntry(S seq) const { return const_cast<SequenceBuffer*>(this)->getEntry(seq); }

		public:
			SequenceBuffer() = default;
			SequenceBuffer(S init) : lowest{init}, next{init} {}

			constexpr static S capacity() noexcept { return N; }
			ENGINE_INLINE S max() const noexcept { return next - 1; }
			ENGINE_INLINE S min() const noexcept { return next - capacity(); }
			ENGINE_INLINE S minValid() const noexcept { return lowest; }
			ENGINE_INLINE S span() const noexcept { return next - lowest; }

			ENGINE_INLINE void clear(S init) {
				memset(&entries, 0, sizeof(entries));
				lowest = init;
				next = init;
			}

			ENGINE_INLINE bool entryAt(S seq) const {
				return getEntry(seq);
			}

			ENGINE_INLINE bool canInsert(S seq) const {
				return !Math::Seq::less(seq, min());
			}

			T& insertNoInit(S seq) {
				// There are a five cases we need to consider.
				// 
				// 1. seq < min
				// 2. min < seq < lowest
				// 3. lowest < seq < max
				// 4. max < seq
				// 5. max << seq
				// 
				//         +_min     +_lowest       +_next
				// ...00000|000000000|10001010110101|00000...
				// Note: max = next - 1
				//
				// The first case is an error/bug.
				// The second and third cases can be handled by simply setting seq as inserted.
				// The fourth case we just advance our window until until max = seq.
				// The last case we are inserting a seq so far ahead that we must clear our entire pre existing range.
				// - We need to do this clear and set behaviour instead of advancing so that we arent repeating work
				// - and looping for a long time when seq much greater than max.
				//
				if (const S n = seq + 1; Math::Seq::greater(n, next)) { // Outside existing range
					S mmin = n - capacity();
					
					static_assert(std::is_unsigned_v<decltype(mmin)>, "wut"); // TODO: rm
					if (Math::Seq::greater(mmin, max())) { // 5. No overlap with existing range
						clear(seq);
						getEntry(seq) = true;
					} else { // 4. Partial overlap with existing range
						// Unset out of bounds entries 
						while (Math::Seq::less(lowest, mmin)) { getEntry(lowest) = false; ++lowest; }
						
						// Advance lowest to next valid
						getEntry(seq) = true;
						while (!getEntry(lowest)) { ++lowest; }
					}

					next = n; // This must be after because of the clear in case 5.
				} else {
					if (Math::Seq::less(seq, lowest)) { // 2. Prepend to existing range
						ENGINE_DEBUG_ASSERT(!Math::Seq::less(seq, min()), "Attempting to insert out of bounds sequence number.");
						lowest = seq;
					} else { // 1. Inside existing range
						// Assert disabled because there are rareish cases where we do this intentionally.
						//ENGINE_DEBUG_ASSERT(getEntry(seq) == false, "Attempting to insert duplicate sequence number.");
					}

					getEntry(seq) = true;
				}

				return get(seq);
			}

			T& insert(S seq) {
				return insertNoInit(seq) = T();
			}

			ENGINE_INLINE void remove(S seq) {
				getEntry(seq) = false;

				while (Math::Seq::less(lowest, next) && !getEntry(lowest)) {
					++lowest;
				}
			}

			ENGINE_INLINE bool contains(S seq) const {
				return getEntry(seq) && !Math::Seq::less(seq, lowest) && Math::Seq::less(seq, next);
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
