#pragma once

// Engine
#include <Engine/Engine.hpp>

#define ENGINE_NET_READ_TO(Msg, Type, Var) \
	if (!Msg.read(&Var)) { \
		ENGINE_WARN("Unable to read " #Var "(" #Type ") from network."); \
		return; \
	}

#define ENGINE_NET_READ(Msg, Type, Var) \
	Type Var; \
	ENGINE_NET_READ_TO(Msg, Type, Var);
	

namespace Engine::Net {
	template<class Self>
	class BitBufferWriter {
		private:
			uint64 bitStore = 0;
			uint64 bitCount = 0;

		public:
			// TODO: don't these need to have retunr values like the other write functions?
			template<int N>
			void writeBits(uint32 t) {
				static_assert(0 < N && N <= 32, "Attempting to write invalid number of bits");

				bitStore |= uint64{t} << bitCount;
				bitCount += N;
				while (bitCount >= 8) {
					self().write<uint8>(static_cast<uint8>(bitStore));
					bitStore >>= 8;
					bitCount -= 8;
				}
			}

			void writeFlushBits() {
				while (bitCount > 0) {
					self().write<uint8>(static_cast<uint8>(bitStore));
					bitStore >>= 8;
					bitCount -= 8;
				}
				bitCount = 0;
			}

		private:
			Self& self() { return static_cast<Self&>(*this); }
	};

	class StaticBufferWriter : public BitBufferWriter<StaticBufferWriter> {
		protected:
			byte* const start = nullptr;
			byte* curr = nullptr;
			byte* const stop = nullptr;

		public:
			StaticBufferWriter(const StaticBufferWriter&) = delete;

			StaticBufferWriter(void* first, void* last)
				: start{static_cast<byte*>(first)}
				, curr{start}
				, stop{static_cast<byte*>(last)} {
			}

			StaticBufferWriter(void* data, uintz sz)
				: start{static_cast<byte*>(data)}
				, curr{start}
				, stop{start + sz} {
			}
			
			template<uintz N>
			StaticBufferWriter(byte (&arr)[N])
				: StaticBufferWriter{arr, N} {
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
			ENGINE_INLINE const byte* cbegin() const noexcept { return start; }
			ENGINE_INLINE const byte* cend() const noexcept { return curr; }

			ENGINE_INLINE void reset() { curr = start; }

			/**
			 * Advance the current position in the buffer without writing to it.
			 */
			ENGINE_INLINE void advance(const int64 count) noexcept { curr += count; }

			/**
			 * Creates a new unrelated BufferWriter using the remaining space in the current BufferWriter.
			 * The created view is unrelated to the aliased view. Writing to one will not update
			 * the position of the other. If you want to emulate this behavior you will need to manually
			 * call `advance`.
			 */
			ENGINE_INLINE StaticBufferWriter alias() noexcept { return {curr, stop}; }

			ENGINE_INLINE_REL bool write(const void* src, int64 sz) {
				ENGINE_DEBUG_ASSERT(sz > 0);
				ENGINE_DEBUG_ASSERT(sz <= capacity(), capacity());

				if (curr + sz > stop) { return false; }
				memcpy(curr, src, sz);
				curr += sz;
				return true;
			}

			template<class T>
			ENGINE_INLINE bool write(const T& data) {
				return write(&data, sizeof(T));
			}
	};

	class DynamicBufferWriter : public BitBufferWriter<DynamicBufferWriter> {
		protected:
			std::vector<byte>* dataStorage = nullptr;

		public:
			DynamicBufferWriter(std::vector<byte>* dataStorage)
				: dataStorage{dataStorage} {
			}

			DynamicBufferWriter(const DynamicBufferWriter&) = delete;

			void reset(std::vector<byte>* newStorage) { dataStorage = newStorage; }
			ENGINE_INLINE void clear() { storage().clear(); }

			ENGINE_INLINE uintz size() const noexcept { return storage().size(); }
			ENGINE_INLINE uintz capacity() const noexcept { return std::numeric_limits<uintz>::max(); }
			ENGINE_INLINE uintz space() const noexcept { return capacity() - size(); }
			ENGINE_INLINE const byte* data() const noexcept { return storage().data(); }
			ENGINE_INLINE byte* data() noexcept { return storage().data(); }

			ENGINE_INLINE byte* begin() noexcept { return data(); }
			ENGINE_INLINE byte* end() noexcept { return data() + size(); }
			ENGINE_INLINE const byte* begin() const noexcept { return const_cast<DynamicBufferWriter*>(this)->begin(); }
			ENGINE_INLINE const byte* end() const noexcept { return const_cast<DynamicBufferWriter*>(this)->end(); }
			ENGINE_INLINE const byte* cbegin() const noexcept { return begin(); }
			ENGINE_INLINE const byte* cend() const noexcept { return end(); }


			ENGINE_INLINE_REL bool write(const void* src, uintz sz) {
				ENGINE_DEBUG_ASSERT(src != nullptr);
				ENGINE_DEBUG_ASSERT(sz > 0);
				ENGINE_DEBUG_ASSERT(sz < 0x7FFF'FFFF); // Not actually a problem, but likely unintended. We dont currently send any data this large.

				const byte* const begp = reinterpret_cast<const byte*>(src);
				const byte* const endp = begp + sz;
				storage().insert(storage().end(), begp, endp);
				return true;
			}

			template<class T>
			ENGINE_INLINE bool write(const T& data) {
				return write(&data, sizeof(T));
			}

		private:
			ENGINE_INLINE std::vector<byte>& storage() const noexcept {
				ENGINE_DEBUG_ASSERT(dataStorage != nullptr);
				return *dataStorage;
			}
	};

	// TODO: move bit packing into base type similar to BitBufferWriter.
	class BufferReader {
		protected:
			const byte* start = nullptr;
			const byte* curr = nullptr;
			const byte* stop = nullptr;
			uint64 bitStore = 0;
			uint64 bitCount = 0;

		public:
			BufferReader() = default;

			//template<auto N>
			//BufferReader(byte (&arr)[N])
			//	: BufferReader{arr, arr + N} {
			//}

			BufferReader(const void* first, const void* last)
				: start{static_cast<const byte*>(first)}
				, curr{start}
				, stop{static_cast<const byte*>(last)} {
			}

			BufferReader(const void* data, int64 sz)
				: start{static_cast<const byte*>(data)}
				, curr{start}
				, stop{start + sz} {
			}
			
			template<auto N>
			ENGINE_INLINE auto reset(byte (&arr)[N]) {
				*this = {arr, arr + N};
			}

			ENGINE_INLINE auto size() const noexcept { return stop - start; }
			ENGINE_INLINE auto capacity() const noexcept { return stop - start; }
			ENGINE_INLINE auto remaining() const noexcept { return stop - curr; }
			ENGINE_INLINE const byte* data() const noexcept { return start; }
			ENGINE_INLINE const byte* peek() const noexcept { return curr; } // TODO: is there a std name for this kind of ptr?
			ENGINE_INLINE void discard() noexcept { curr = stop; }
			ENGINE_INLINE void resize(int64 sz) noexcept { stop = start + sz; }

			ENGINE_INLINE const byte* begin() const noexcept { return start; }
			ENGINE_INLINE const byte* end() const noexcept { return stop; }
			ENGINE_INLINE const byte* cbegin() const noexcept { return start; }
			ENGINE_INLINE const byte* cend() const noexcept { return stop; }

			/**
			 * Creates a new unrelated BufferReader using the remaining space in the current BufferReader.
			 * The created view is unrelated to the aliased view. Writing to one will not update
			 * the position of the other. If you want to emulate this behavior you will need to manually
			 * call `advance`.
			 */
			ENGINE_INLINE BufferReader alias() noexcept { return {curr, stop}; }

			/**
			 * Read the given number of bytes.
			 * @return A pointer to the read bytes or nullptr if unable to read the given number of bytes.
			 */
			ENGINE_INLINE const byte* read(int64 sz) noexcept {
				ENGINE_DEBUG_ASSERT(sz > 0);
				if (curr + sz > stop) { return nullptr; }
				const auto tmp = curr;
				curr += sz;
				return tmp;
			}

			/**
			 * Copy the given number of bytes to @p out.
			 * @return True if successful. False if unable to read the given number of bytes.
			 */
			ENGINE_INLINE bool read(int64 sz, void* out) noexcept {
				ENGINE_DEBUG_ASSERT(curr + sz <= stop, "Insufficient space remaining to read");
				ENGINE_DEBUG_ASSERT(bitCount == 0 && bitStore == 0, "Incomplete read of packed bits");
				if (curr + sz > stop) { return false; }
				memcpy(out, curr, sz);
				curr += sz;
				return true;
			}

			/**
			 * Copy a single @p T into @p out.
			 * @return True if successful. False if unable to read.
			 */
			template<class T>
			ENGINE_INLINE bool read(T* out) noexcept {
				static_assert(!std::is_const_v<T>, "BufferReader is unable to read to a const variable.");
				return read(sizeof(T), out);
			}

			template<uint64 N, std::integral T>
			bool read(T* out) noexcept {
				static_assert(0 < N && N <= 32, "Attempting to read invalid number of bits");
				static_assert(sizeof(T) * CHAR_BIT >= N, "Invalid output type for given number of bits");

				while (bitCount < N) {
					bitStore |= uint64{*curr} << bitCount;
					bitCount += 8;
					curr += 1;
				}

				ENGINE_DEBUG_ASSERT(curr <= stop, "Read to many bits from buffer.");

				constexpr uint64 mask = (1ull << N) - 1ull;
				*out = static_cast<T>(bitStore & mask);
				bitStore >>= N;
				bitCount -= N;
				return true;
			}

			template<uint64 N, class T>
			T read() noexcept {
				T res = 0;
				read<N>(&res);
				return res;
			}

			ENGINE_INLINE void readFlushBits() noexcept {
				bitStore = 0;
				bitCount = 0;
			}
	};
}
