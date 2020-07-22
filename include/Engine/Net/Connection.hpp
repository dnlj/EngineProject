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
	inline constexpr int32 MAX_MESSAGE_SIZE2 = sizeof(Packet2::body) - sizeof(MessageHeader);

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
		None		= 0 << 0,
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

			const IPv4Address addr;
			ChannelInfo channelInfo[sizeof...(Cs)];
			PacketInfo packetInfo[4];
			NodePtr activePackets[4];
			NodePtr pool;
			PacketNode* currNode = nullptr;
			Engine::Clock::TimePoint lastRecvTime;

			struct {
				/** The time the message was received */
				Engine::Clock::TimePoint time;

				/** The first byte in the recv packet */
				const byte* first;

				/** One past the last byte in the recv packet */
				const byte* last;

				/** The current position in the recv packet */
				const byte* curr;

				/** One past the last byte in the current message */
				const byte* msgLast;
			} rdat;

			template<class C>
			constexpr static ChannelId getChannelId() {
				return Meta::IndexOf<C, Cs>::value;
			}

			NodePtr getOrAllocPacketFromPool() {
				if (!pool) {
					auto ptr = std::make_unique<PacketNode>();
					ptr->clear();
					return ptr;
				}
				auto old = std::move(pool);
				pool.swap(old->next);
				old->clear();
				return old;
			}

			PacketNode& getOrAllocPacket(ChannelFlags flags) {
				auto& node = activePackets[static_cast<int32>(flags)];
				if (!node) {
					node = getOrAllocPacketFromPool();
				}

				auto* curr = node.get();
				while(curr->next) { curr = curr->next.get(); }
				return *curr;
			}
			
		public:
			// TODO: ping
			// TODO: packet loss
			// TODO: bandwidth in/out. Could do per packet type (4)

			Connection2(IPv4Address addr, Engine::Clock::TimePoint time) : addr{addr} {
				rdat.time = time;
			}

			const auto& address() const { return addr; }

			// TODO: doc
			void recv(const Packet2& pkt, int32 sz, Engine::Clock::TimePoint time) {
				rdat.time = time;
				rdat.first = pkt.head;
				rdat.last = rdat.first + sz;
				rdat.curr = pkt.body;
				rdat.msgLast = rdat.curr;

				// TODO: packet header info
			}

			auto recvTime() const { return rdat.time; }

			// TODO: doc
			const MessageHeader* recvNext() {
				if (rdat.curr >= rdat.last) { return nullptr; }
				ENGINE_DEBUG_ASSERT(rdat.curr == rdat.msgLast, "Incomplete read of network message");
				const auto* hdr = read<MessageHeader>();
				ENGINE_DEBUG_ASSERT(hdr->size <= MAX_MESSAGE_SIZE2, "Invalid network message length");
				rdat.msgLast = rdat.curr + hdr->size;
				return hdr;
			}

			/**
			 * The number of bytes remaining in the current recv message.
			 */
			auto recvMsgSize() const { return static_cast<int32>(rdat.msgLast - rdat.curr); }

			/**
			 * The number of byte reminaing in the current recv packet.
			 */
			auto recvSize() const { return static_cast<int32>(rdat.last - rdat.curr); }

			/**
			 * Reads a specific number of bytes from the current message.
			 */
			const void* read(size_t sz) {
				ENGINE_DEBUG_ASSERT(rdat.curr + sz <= rdat.last, "Insufficient space remaining to read");
				if (rdat.curr + sz > rdat.last) { return nullptr; }

				const void* temp = rdat.curr;
				rdat.curr += sz;
				// TODO: bandwidth recv
				return temp;
			}
			
			/**
			 * Reads an object from the current message.
			 */
			template<class T>
			decltype(auto) read() {
				if constexpr (std::is_same_v<T, char*> || std::is_same_v<T, const char*>) {
					return reinterpret_cast<const char*>(read(strlen(rdat.curr) + 1));
				} else {
					return reinterpret_cast<const T*>(read(sizeof(T)));
				}
			}

			void send(UDPSocket& sock) {
				for (int32 i = 0; i < std::size(activePackets); ++i) {
					auto& p = activePackets[i];
					while (p) {
						auto& info = packetInfo[i];
						p->packet.setSeqNum(info.nextSeq);
						++info.nextSeq;

						const auto sz = static_cast<int32>(p->last - p->packet.head);
						if (sz > sizeof(p->packet.head)) {
							sock.send(p->packet.head, sz, addr);
						}

						NodePtr old = std::move(p);
						p.swap(old->next);
						old->next.swap(pool);
						pool.swap(old);
					}
				}
			}

			template<class C>
			void msgBegin(MessageType type, const C&) {
				static_assert((std::is_same_v<C, Cs> || ...), "Invalid network connection channel");
				// TODO: check that currNode is empty
				ENGINE_DEBUG_ASSERT(currNode == nullptr, "Attempting to begin new message without ending the previous message.");
				auto& node = getOrAllocPacket(C::flags);
				currNode = &node;

				// TODO: does this write to pkt.head? check
				write(MessageHeader{
					.type = type,
					.channel = Channel::UNRELIABLE, // TODO: getChannelId<C>()
					.sequence = 0, // TODO: impl
				});
			}

			void msgEnd() {
				auto* head = reinterpret_cast<MessageHeader*>(currNode->curr);
				head->size = static_cast<decltype(head->size)>(currNode->size() - sizeof(*head));
				currNode->curr = currNode->last;
				currNode = nullptr;
			}

			/**
			 * Writes a specific number of bytes to the current message.
			 */
			void write(const void* t, size_t sz) {
				ENGINE_DEBUG_ASSERT(currNode != nullptr, "No network message active.");
				//ENGINE_LOG("Write: ", sz); __debugbreak();
				ENGINE_DEBUG_ASSERT(sz <= MAX_MESSAGE_SIZE2, "Message data exceeds MAX_MESSAGE_SIZE = ", MAX_MESSAGE_SIZE2, " bytes.");

				if (currNode->last + sz <= currNode->packet.body + sizeof(currNode->packet.body)) {
					memcpy(currNode->last, t, sz);
					currNode->last += sz;
				} else {
					ENGINE_DEBUG_ASSERT(currNode->next == nullptr);
					auto old = currNode;
					currNode->next = getOrAllocPacketFromPool();
					currNode = currNode->next.get();
					// TODO: set currNode ptrs? i think
					ENGINE_WARN("Network message rollover. This code is untested.");
					write(old->curr, old->size());
					write(t, sz);
					old->last = old->curr;
				}
			}

			/**
			 * Writes an object to the current message.
			 */
			template<class T>
			void write(const T& t) { write(&t, sizeof(T)); };

			/**
			 * Writes a string to the current message.
			 */
			void write(const char* t) { write(t, strlen(t) + 1); }
	};
}
