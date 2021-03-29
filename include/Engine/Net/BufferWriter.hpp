#pragma once

// Engine
#include <Engine/Engine.hpp>


namespace Engine::Net {
	class BufferWriter { // TODO: move into Engine instead of Engine::Net
		private:
			byte* start = nullptr;
			byte* curr = nullptr;
			byte* stop = nullptr;
			int bitCount = 0;
			uint64 bitStore = 0;

		public:
			BufferWriter() = default;

			template<auto N>
			BufferWriter(byte (&arr)[N])
				: BufferWriter{arr, N} {
			}
			
			BufferWriter(void* data, int64 sz)
				: start{static_cast<byte*>(data)}
				, curr{start}
				, stop{start + sz} {
			}

			ENGINE_INLINE auto size() const noexcept { return curr - start; }
			ENGINE_INLINE auto capacity() const noexcept { return stop - start; }
			ENGINE_INLINE auto space() const noexcept { return capacity() - size(); }
			ENGINE_INLINE const byte* data() const noexcept { return start; }
			ENGINE_INLINE byte* data() noexcept { return start; }

			ENGINE_INLINE byte* begin() noexcept { return start; }
			ENGINE_INLINE byte* end() noexcept { return curr; }
			ENGINE_INLINE const byte* begin() const noexcept { return start; }
			ENGINE_INLINE const byte* end() const noexcept { return curr; }
			ENGINE_INLINE const byte* cbegin() noexcept { return start; }
			ENGINE_INLINE const byte* cend() noexcept { return curr; }
			ENGINE_INLINE const byte* cbegin() const noexcept { return start; }
			ENGINE_INLINE const byte* cend() const noexcept { return curr; }

			/**
			 * Advance the current position in the buffer without writing to it.
			 */
			ENGINE_INLINE void advance(const int64 count) noexcept { curr += count; }

			/**
			 * Creates a new unrelated BufferWriter using the remaining space in the current BufferWriter.
			 * The created writer is unrelated to the aliased writer. Writing to one will not update
			 * the position of the other. If you want to emulate this behavior you will need to manually
			 * call `advance`.
			 */
			ENGINE_INLINE auto alias() const noexcept { return BufferWriter{curr, space()}; }

			ENGINE_INLINE bool write(const void* src, int64 sz) {
				ENGINE_DEBUG_ASSERT(sz > 0);
				ENGINE_DEBUG_ASSERT(sz <= capacity());

				if (curr + sz > stop) { return false; }
				memcpy(curr, src, sz);
				curr += sz;
				return true;
			}

			template<class T>
			ENGINE_INLINE bool write(const T& data) {
				return write(&data, sizeof(T));
			}
			
			// TODO: add version for more than 32 bits?
			template<int N>
			void write(uint32 t) {
				bitStore |= uint64{t} << bitCount;
				bitCount += N;
				while (bitCount >= 8) {
					write(static_cast<uint8>(bitStore));
					bitStore >>= 8;
					bitCount -= 8;
				}
			}

			void writeFlushBits() {
				while (bitCount > 0) {
					write(static_cast<uint8>(bitStore));
					bitStore >>= 8;
					bitCount -= 8;
				}
				bitCount = 0;
			}
	};
}
