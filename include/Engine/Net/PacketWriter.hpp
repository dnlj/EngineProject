#pragma once

// Engine
#include <Engine/Net/PacketNode.hpp>


namespace Engine::Net {
	class BufferWriter {
		private:
			byte* curr = nullptr;
			byte* const start = nullptr;
			byte* const stop = nullptr;
			int bitCount = 0;
			uint64 bitStore = 0;

		public:
			// TODO: couldnt this just be done with CTAD?
			template<auto N>
			BufferWriter(byte (&arr)[N])
				: BufferWriter{arr, N} {
			}
			
			BufferWriter(void* data, int64 size)
				: start{static_cast<byte*>(data)}
				, curr{start}
				, stop{start + size} {
			}

			ENGINE_INLINE auto size() const noexcept { return curr - start; }
			ENGINE_INLINE auto capacity() const noexcept { return stop - start; }
			ENGINE_INLINE auto space() const noexcept { return size() - capacity(); }
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

			ENGINE_INLINE bool write(const void* src, int64 sz) {
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

	class PacketWriter {
		private:
			using NodePtr = std::unique_ptr<PacketNode>;
			NodePtr pool = nullptr;
			NodePtr first = nullptr;
			PacketNode* last = nullptr;
			SeqNum nextSeq = 0;
			int bitCount = 0;
			uint64 bitStore = 0;

			NodePtr getOrAllocPacketFromPool() {
				if (!pool) {
					auto ptr = std::make_unique<PacketNode>();
					ptr->clear();
					ptr->packet.setProtocol(protocol); // TODO: implicit protocol
					pool = std::move(ptr);
				}
				auto old = std::move(pool);
				pool.swap(old->next);
				old->clear();
				old->packet.setSeqNum(nextSeq++);
				return old;
			}

			PacketNode& getOrAllocPacket() {
				if (!last) {
					ENGINE_DEBUG_ASSERT(!first);
					first = getOrAllocPacketFromPool();
					last = first.get();
				}
				return *last;
			}

		public:
			auto getNextSeq() const { return nextSeq; }

			void ensurePacketAvailable() {
				getOrAllocPacket();
			}

			void advance() {
				last->curr = last->last;
			}

			PacketNode* pop() {
				if (!first) { return nullptr; }
				if (last == first.get()) { last = nullptr; }
				pool.swap(first->next);
				first.swap(pool);

				// TODO: need to update last. maybe this is why we use msgBegin/msgEnd

				return pool.get();
			}

			PacketNode* back() {
				return last;
			}

			int32 space() const noexcept {
				const auto stop = last->packet.body + sizeof(last->packet.body);
				const auto curr = last->last;
				return static_cast<int32>(stop - curr);
			}

			/**
			 * Writes a specific number of bytes to the current message.
			 */
			void* write(const void* t, size_t sz) {
				ENGINE_DEBUG_ASSERT(last != nullptr, "No network message active.");
				ENGINE_DEBUG_ASSERT(sz <= sizeof(Packet::body), "Message data ", sz," exceeds sizeof(Packet::body) = ", sizeof(Packet::body), " bytes.");

				if (last->last + sz <= last->packet.body + sizeof(last->packet.body)) {
					memcpy(last->last, t, sz);
					last->last += sz;
				} else {
					ENGINE_DEBUG_ASSERT(last->next == nullptr);
					auto old = last;
					last->next = getOrAllocPacketFromPool();
					last = last->next.get();
					write(old->curr, old->size());
					write(t, sz);
					old->last = old->curr;
				}

				return last->last - sz;
			}

			/**
			 * Writes an object to the current message.
			 */
			template<class T>
			auto write(const T& t) {
				static_assert(!std::is_pointer_v<T>, "Can't write pointers to packet.");
				return static_cast<T*>(write(&t, sizeof(T)));
			};

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
