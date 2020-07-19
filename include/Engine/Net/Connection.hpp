#pragma once

// STD
#include <memory>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Net/AckData.hpp>
#include <Engine/Net/IPv4Address.hpp>
#include <Engine/Net/MessageHeader.hpp>
#include <Engine/Net/Net.hpp>
#include <Engine/Net/PacketWriter.hpp>
#include <Engine/Net/PacketReader.hpp>
#include <Engine/Clock.hpp>
#include <Engine/StaticVector.hpp>


namespace Engine::Net {
	class PacketNode {
		public:
			PacketNode(const PacketNode&) = delete; // Would break curr/last ptrs
			std::unique_ptr<PacketNode> next = nullptr;
			byte* curr;
			byte* last;
			Packet packet;

			void clear() {
				curr = packet.data;
				last = curr;
			}

			int32 size() { return static_cast<int32>(last - curr); }
	};

	using ChannelId = uint8;

	enum class ChannelFlags : ChannelId {
		Reliable	= 1 << 0,
		Ordered		= 1 << 1,
	};

	ENGINE_INLINE constexpr ChannelFlags operator|(ChannelFlags a, ChannelFlags b) {
		return static_cast<ChannelFlags>(static_cast<ChannelId>(a) | static_cast<ChannelId>(b));
	}

	template<ChannelFlags Flags>
	class Channel2 {
		public:
			constexpr static auto flags = Flags;
	};

	template<class... Cs>
	class Connection2 {
		private:
			struct ChannelInfo {
				SequenceNumber nextSeq;
			};

			using NodePtr = std::unique_ptr<PacketNode>;
			ChannelInfo channelInfo[sizeof...(Cs)];
			NodePtr activePackets[4];
			NodePtr pool;
			SequenceNumber nextSeq = 0;
			PacketNode* currPacket;

			NodePtr getOrAllocPacketFromPool() {
				if (!pool) { return std::make_unique<PacketNode>(); }
				auto old = std::move(pool);
				pool = std::move(pool->next);
				old->next = nullptr;
				return old;
			}

			PacketNode& getOrAllocPacket(ChannelFlags flags) {
				auto& node = active[static_cast<int32>(flags)];
				if (!node) {
					node = getOrAllocPacketFromPool();
					node->clear();
				}

				auto* curr = node.get();
				while(curr->next) { curr = curr->next; }
				return *curr;
			}
			
		public:
			// TODO: ping
			// TODO: packet loss
			// TODO: bandwidth in/out
			
			template<class C>
			void beginMessage(MessageType type, const C&) {
				auto& node = getOrAllocPacket(C::flags);
				currPacket = &node;
				write(MessageHeader{
					.type = type,
					.channel = getChannelId<C>(),
					.size = 0,
					.sequence = 0, // TODO: impl
				});
			}

			void endMessage() {
				auto* head = reinterpret_cast<MessageHeader*>(currPacket->curr);
				head->size = currPacket->size();
				currPacket->curr = currPacket->last;
				currPacket = nullptr;
			}

			/**
			 * Writes a specific number of bytes to the current message.
			 */
			void write(const void* t, size_t sz) {
				ENGINE_DEBUG_ASSERT(sz <= MAX_MESSAGE_SIZE, "Message data exceeds MAX_MESSAGE_SIZE = ", MAX_MESSAGE_SIZE, " bytes.");

				if (currPacket->last + sz <= currPacket->data + sizeof(packet->data)) {
					memcpy(currPacket->last, t, sz);
					currPacket->last += sz;
				} else {
					ENGINE_DEBUG_ASSERT(currPacket->next == nullptr);
					auto old = currPacket;
					currPacket->next = getOrAllocPacketFromPool();
					currPacket = currPacket->next;
					write(old->curr, old->last - old->curr);
					write(t, sz);
					old->last = old->curr;
				}
			}

			/**
			 * Writes an object to the current message.
			 */
			template<class T>
			void write(const T& t) { write(&t, sizeof(T)); };

	};

	// TODO: rm
	struct MyChannel_1_Tag : Channel2<ChannelFlags::Reliable | ChannelFlags::Ordered> {} constexpr MyChannel_1;
	struct MyChannel_2_Tag : Channel2<ChannelFlags::Reliable | ChannelFlags::Ordered> {} constexpr MyChannel_2;
	using MyConnection = Connection2<MyChannel_1_Tag, MyChannel_2_Tag>;


	class Connection {
		public:
			PacketReader reader;
			PacketWriter writer;
			Clock::TimePoint lastMessageTime;
			const Clock::TimePoint connectTime;

		public:
			Connection(UDPSocket& sock, IPv4Address addr = {}, Clock::TimePoint lastMessageTime = {});
			Connection(const Connection&) = delete;
			Connection(const Connection&&) = delete;

			void writeRecvAcks(Channel ch);

			/**
			 * Gets the most recently associated address.
			 * Set from either #reset or #recv.
			 */
			IPv4Address address() const;
	};
}
