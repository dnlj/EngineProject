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
	using SeqNum = uint32;

	constexpr inline int32 MAX_PACKET_SIZE = 512;
	class Packet2 {
		public:
			byte head[6] = {0b0110, 0b1001, 0b1001, 0b0110};
			byte body[MAX_PACKET_SIZE - sizeof(head)];

		public:
			auto& getProtocol() { return *reinterpret_cast<uint16*>(&head[0]); }
			auto& getProtocol() const { return *reinterpret_cast<const uint16*>(&head[0]); }
			void setProtocol(uint16 p) { getProtocol() = p; }

			auto& getSeqNum() { return *reinterpret_cast<SeqNum*>(&head[2]); }
			auto& getSeqNum() const { return *reinterpret_cast<const SeqNum*>(&head[2]); }
			void setSeqNum(SeqNum n) { getSeqNum() = n; }
	};
	static_assert(sizeof(Packet2) == MAX_PACKET_SIZE);

	class PacketNode {
		public:
			PacketNode() = default;
			PacketNode(const PacketNode&) = delete; // Would break curr/last ptrs
			std::unique_ptr<PacketNode> next = nullptr;
			byte* curr;
			byte* last;

			Packet2 packet;

			void clear() {
				curr = packet.body;
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
			using NodePtr = std::unique_ptr<PacketNode>;

			struct ChannelInfo {
				SequenceNumber nextSeq;
			};

			struct PacketInfo {
				SequenceNumber nextSeq;
				// TODO: only need this for reliable flag
				SequenceNumber lastAck;
				NodePtr packets[64]; // TODO: if not nullptr then unacked
			};

			ChannelInfo channelInfo[sizeof...(Cs)];
			PacketInfo packetInfo[4];
			NodePtr activePackets[4];
			NodePtr pool;
			PacketNode* currNode;
			const IPv4Address addr;

			template<class C>
			constexpr static ChannelId getChannelId() {
				return Meta::IndexOf<C, Cs>::value;
			}

			NodePtr getOrAllocPacketFromPool() {
				if (!pool) { return std::make_unique<PacketNode>(); }
				auto old = std::move(pool);
				pool = std::move(pool->next);
				old->next = nullptr;
				return old;
			}

			PacketNode& getOrAllocPacket(ChannelFlags flags) {
				auto& node = activePackets[static_cast<int32>(flags)];
				if (!node) {
					node = getOrAllocPacketFromPool();
					node->clear();
				}

				auto* curr = node.get();
				while(curr->next) { curr = curr->next.get(); }
				return *curr;
			}
			
		public:
			// TODO: ping
			// TODO: packet loss
			// TODO: bandwidth in/out. Could do per packet type (4)

			void send(UDPSocket& sock) {
				for (int32 i = 0; i < std::size(activePackets); ++i) {
					PacketNode* p = activePackets[i];
					if (!p) { continue; }
					while (p) {
						auto& info = packetInfo[i];
						p->packet.setSeqNum(info.nextSeq);
						++info.nextSeq;

						// TODO: send packet
						const auto sz = p->packet.head - p->last;
						sock.send(p->packet.head, sz, addr);

						auto old = p;
						p = p->next;
						old->next = pool;
						pool = old;
					}
				}
			}
			
			template<class C>
			void beginMessage(MessageType type, const C&) {
				auto& node = getOrAllocPacket(C::flags);
				currNode = &node;
				write(MessageHeader{
					.type = type,
					.channel = Channel::UNRELIABLE, // TODO: getChannelId<C>()
					.sequence = 0, // TODO: impl
				});
			}

			void endMessage() {
				auto* head = reinterpret_cast<MessageHeader*>(currNode->curr);
				head->size = currNode->size();
				currNode->curr = currNode->last;
				currNode = nullptr;
			}

			/**
			 * Writes a specific number of bytes to the current message.
			 */
			void write(const void* t, size_t sz) {
				ENGINE_DEBUG_ASSERT(sz <= MAX_MESSAGE_SIZE, "Message data exceeds MAX_MESSAGE_SIZE = ", MAX_MESSAGE_SIZE, " bytes.");

				if (currNode->last + sz <= currNode->packet.body + sizeof(currNode->packet.body)) {
					memcpy(currNode->last, t, sz);
					currNode->last += sz;
				} else {
					ENGINE_DEBUG_ASSERT(currNode->next == nullptr);
					auto old = currNode;
					currNode->next = getOrAllocPacketFromPool();
					currNode = currNode->next.get();
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
