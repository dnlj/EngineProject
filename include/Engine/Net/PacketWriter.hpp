#pragma once

// Engine
#include <Engine/Net/PacketNode.hpp>


namespace Engine::Net {
	class PacketWriter {
		private:
			constexpr static uint16 protocol = 0b0'0110'1001'1001'0110;

			using NodePtr = std::unique_ptr<PacketNode>;
			NodePtr pool = nullptr;
			NodePtr first = nullptr;
			PacketNode* last = nullptr;

			NodePtr getOrAllocPacketFromPool() {
				if (!pool) {
					auto ptr = std::make_unique<PacketNode>();
					ptr->clear();
					ptr->packet.setProtocol(protocol); // TODO: implicit protocol
					return ptr;
				}
				auto old = std::move(pool);
				pool.swap(old->next);
				old->clear();
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
			void ensurePacketAvailable() {
				getOrAllocPacket();
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

			/**
			 * Writes a specific number of bytes to the current message.
			 */
			void* write(const void* t, size_t sz) {
				ENGINE_DEBUG_ASSERT(last != nullptr, "No network message active.");
				ENGINE_DEBUG_ASSERT(sz <= MAX_MESSAGE_SIZE, "Message data exceeds MAX_MESSAGE_SIZE = ", MAX_MESSAGE_SIZE, " bytes.");

				if (last->last + sz <= last->packet.body + sizeof(last->packet.body)) {
					memcpy(last->last, t, sz);
					last->last += sz;
				} else {
					ENGINE_DEBUG_ASSERT(last->next == nullptr);
					auto old = last;
					last->next = getOrAllocPacketFromPool();
					last = last->next.get();
					// TODO: set curr ptrs? i think
					ENGINE_WARN("Network message rollover. This code is untested.");
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
			auto write(const T& t) { return static_cast<T*>(write(&t, sizeof(T))); };

			/**
			 * Writes a string to the current message.
			 */
			auto write(const char* t) { return static_cast<char*>(write(t, strlen(t) + 1)); }
	};
}
