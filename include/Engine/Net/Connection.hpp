#pragma once

// STD
#include <memory>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Net/MessageHeader.hpp>
#include <Engine/Net/IPv4Address.hpp>
#include <Engine/Net/UDPSocket.hpp>
#include <Engine/Net/Net.hpp>
#include <Engine/Net/Packet.hpp>
#include <Engine/StaticVector.hpp>
#include <Engine/Bitset.hpp>
#include <Engine/Clock.hpp>
#include <Engine/Utility/Utility.hpp>


namespace Engine::Net {
	class PacketNode {
		public:
			PacketNode() = default;
			PacketNode(const PacketNode&) = delete; // Would break curr/last ptrs
			std::unique_ptr<PacketNode> next = nullptr;
			byte* curr;
			byte* last;

			Packet packet;

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

	ENGINE_INLINE constexpr ChannelFlags operator&(ChannelFlags a, ChannelFlags b) {
		return static_cast<ChannelFlags>(static_cast<ChannelId>(a) & static_cast<ChannelId>(b));
	}

	ENGINE_INLINE ChannelFlags& operator++(ChannelFlags& a) {
		++reinterpret_cast<ChannelId&>(a);
		return a;
	}

	template<ChannelFlags Flags>
	class Channel {
		public:
			constexpr static auto flags = Flags;
	};

	template<class... Cs>
	class Connection {
		private:
			using NodePtr = std::unique_ptr<PacketNode>;

			constexpr static uint16 protocol = 0b0'0110'1001'1001'0110;

			const IPv4Address addr = {};
			SeqNum nextPacketSeqUnrel = 0;
			SeqNum nextPacketSeqRel = 0;

			struct UnreliableAckData {
				Engine::Clock::TimePoint sendTime;
				Engine::Clock::TimePoint recvTime;
			};
			/** Data for the last N unreliable packets we have sent. */
			UnreliableAckData unrelAckData[AckBitset::size()] = {};

			/** The latest packet seq num we have received */
			SeqNum lastRecvAckUnrel = {};

			/** Acks for the prev N packets before lastRecvAckUnrel */
			AckBitset recvAcksUnrel = {};

			/** The next ack we are expecting */
			SeqNum nextSentAck = 0;

			/** Data for the last N reliable packets we have sent */
			NodePtr unacked[AckBitset::size()];

			/** The next seq num we are expecting to receive */
			SeqNum nextRecvAck = 0;

			/** The acks for the next N seq num we are expecting to receive */
			AckBitset recvAcks = {};

			/** Packets to send next send() */
			NodePtr unsentPackets[2];

			/** Unused packets */
			NodePtr pool;

			/** The current packet we are writing to */
			PacketNode* currNode = nullptr;

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
			constexpr static ChannelId getChannelId() { return Meta::IndexOf<C, Cs...>::value; }

			NodePtr getOrAllocPacketFromPool() {
				if (!pool) {
					auto ptr = std::make_unique<PacketNode>();
					ptr->clear();
					ptr->packet.setProtocol(protocol);
					return ptr;
				}
				auto old = std::move(pool);
				pool.swap(old->next);
				old->clear();
				return old;
			}

			template<class C>
			PacketNode& getOrAllocPacket() {
				constexpr bool rel = static_cast<bool>(C::flags & ChannelFlags::Reliable);
				auto& node = unsentPackets[rel];
				if (!node) {
					node = getOrAllocPacketFromPool();
					node->packet.setReliable(rel);
				}
				auto* curr = node.get();
				while(curr->next) { curr = curr->next.get(); }
				return *curr;
			}
			
			ENGINE_INLINE constexpr static SeqNum seqToIndex(SeqNum seq) {
				constexpr auto maxUnacked = AckBitset::size();
				static_assert(Engine::Utility::isPowerOfTwo(maxUnacked));
				return seq & (maxUnacked - 1);
			}

			void freeAck(SeqNum seq) {
				auto& node = unacked[seqToIndex(seq)];
				if (!node) { return; }
				ENGINE_DEBUG_ASSERT(node->next == nullptr);
				node->next = std::move(pool);
				pool = std::move(node);
			}

		public:
			// TODO: ping
			// TODO: jitter
			// TODO: packet loss
			// TODO: bandwidth in/out. Could do per packet type (4)

			Connection(IPv4Address addr, Engine::Clock::TimePoint time) : addr{addr} {
				rdat.time = time;
			}

			const auto& address() const { return addr; }

			[[nodiscard]]
			bool recv(const Packet& pkt, int32 sz, Engine::Clock::TimePoint time) {
				if (pkt.getProtocol() != protocol) {
					ENGINE_WARN("Incorrect network protocol");
					return false;
				}

				rdat.time = time;
				rdat.first = pkt.head;
				rdat.last = rdat.first + sz;
				rdat.curr = pkt.body;
				rdat.msgLast = rdat.curr;

				// TODO: packet header info

				const auto& init = pkt.getInitAck();
				const auto& acks = pkt.getAcks();
				const auto seq = pkt.getSeqNum();

				if (pkt.getReliable()) {
					// TODO: move into func
					if (seq < nextRecvAck || seq >= (nextRecvAck + recvAcks.size())) { return false; }
	
					if (const auto i = seqToIndex(seq); recvAcks.test(i)) {
						return false;
					} else {
						recvAcks.set(i);
					}

					for (SeqNum i; recvAcks.test(i = seqToIndex(nextRecvAck));) {
						recvAcks.reset(i);
						++nextRecvAck;
					}

					// TODO: updateSentAcks(init, acks);
				} else {
					{ // TODO: move into func
						int32 diff = seq - lastRecvAckUnrel;
						if (diff > 0) {
							lastRecvAckUnrel = seq;
							recvAcksUnrel << diff;
							recvAcksUnrel.set(0);
						} else if (diff < 0) {
							diff = -diff;
							if (diff < AckBitset::size()) {
								recvAcksUnrel.set(diff);
							}
						} // If equal then it should have been already acked
					}

					{// TODO: move into func
						// Unsigned subtraction is fine here since it would still be > max size
						const auto lastSent = nextPacketSeqUnrel - 1;
						const auto diff = lastSent - init;
						if (diff <= static_cast<SeqNum>(AckBitset::size())) {
							auto curr = lastSent - acks.size();
							const auto stop = init;

							for (;curr <= stop; ++curr) {
								if (!acks.test(init - curr)) { continue; }

								const auto i = seqToIndex(curr);
								auto& data = unrelAckData[i];
								if (data.recvTime != Engine::Clock::TimePoint{}) { continue; }

								data.recvTime = time;

								ENGINE_LOG("RECV ACK: ", curr, " in ",
									Engine::Clock::Seconds{data.recvTime - data.sendTime}.count(), "s"
								);
							}
						}
					}
				}


				return true;
			}

			/**
			 * Updates the acks for packets this connection has sent.
			 * @param first The sequence number of the first ack.
			 * @params acks The bitset indicating if packet `first + i` has been received.
			 */
			void updateSentAcks(SeqNum first, AckBitset acks) {
				// TODO: need to guard against `first > nextPacketSeqRel`
				if (first < nextSentAck) { ENGINE_LOG("================== OH BOY! =================="); }
				// TODO: if (first >= nextPacketSeqRel) { return; }
				while (nextSentAck < first) {
					freeAck(nextSentAck);
					++nextSentAck;
				}

				// The 0th elem should always be false or else first would be larger
				const auto TEMP_RM = acks.test(seqToIndex(first));
				ENGINE_DEBUG_ASSERT(TEMP_RM == false, "The first elem of acks should always be zero.");
				ENGINE_DEBUG_ASSERT(nextSentAck <= first + acks.size(), "nextSentAck is to large. (or first is to small)");

				// This should be safe since `nextSentAck >= first` always. (because of above loop)
				const auto start = nextSentAck;
				const auto stop = std::min(first + acks.size(), nextPacketSeqRel);
				for (auto i = start; i < stop; ++i) {
					// TODO: based on recv above. acks isnt frist + 0, 1, 2 etc. it uses seqToIndex starting at first.
					if (acks.test(seqToIndex(i))) { // TODO: could incorperate seqToIndex into start/stop
						freeAck(i);
					}
				}
			}

			auto getRecvNextAck() const { return nextRecvAck; }
			auto getRecvAcks() const { return recvAcks; }

			auto recvTime() const { return rdat.time; }

			/**
			 * Read the next message from the packet set by recv.
			 */
			const MessageHeader* recvNext() {
				if (rdat.curr >= rdat.last) { return nullptr; }
				ENGINE_DEBUG_ASSERT(rdat.curr == rdat.msgLast, "Incomplete read of network message");
				const auto* hdr = read<MessageHeader>();
				ENGINE_DEBUG_ASSERT(hdr->size <= MAX_MESSAGE_SIZE, "Invalid network message length");
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
					return reinterpret_cast<const char*>(
						read(strlen(reinterpret_cast<const char*>(rdat.curr)) + 1)
					);
				} else {
					return reinterpret_cast<const T*>(read(sizeof(T)));
				}
			}

			void send(UDPSocket& sock) {
				const auto now = Engine::Clock::now();
				{ // Unreliable
					auto& node = unsentPackets[0];
					while (node) {
						const auto i = seqToIndex(nextPacketSeqUnrel);
						auto& data = unrelAckData[i];

						// TODO: rm - debug
						if (data.recvTime == Engine::Clock::TimePoint{}) {
							ENGINE_WARN("NO ACK FOR ", nextPacketSeqUnrel - 64);
						}

						data = {
							.sendTime = now,
						};

						node->packet.setSeqNum(nextPacketSeqUnrel++);
						node->packet.setInitAck(lastRecvAckUnrel);
						node->packet.setAcks(recvAcksUnrel);

						const auto sz = static_cast<int32>(node->last - node->packet.head);
						sock.send(node->packet.head, sz, addr);

						NodePtr old = std::move(node);
						node.swap(old->next);
						old->next.swap(pool);
						pool.swap(old);
					}
				}

				{ // Reliable
					auto& node = unsentPackets[1];
					while (node) {
						auto& store = unacked[seqToIndex(nextPacketSeqRel)];

						if (store == nullptr) {
							node->packet.setSeqNum(nextPacketSeqRel++);

							const auto sz = static_cast<int32>(node->last - node->packet.head);
							sock.send(node->packet.head, sz, addr);

							store = std::move(node);
							node.swap(store->next);
						} else {
							// TODO: to many unacked
							ENGINE_WARN("TODO: to many unacked packets");

							for (auto i = nextSentAck; i < nextPacketSeqRel; ++i) {
								auto* node = unacked[seqToIndex(i)].get();
								std::cout << i << " " << node << " " << (node ? node->packet.getSeqNum() : 0) << "\n";
							}

							__debugbreak();
							break;
						}
					}
				}
			}

			void sendUnacked(UDPSocket& sock) {
				ENGINE_DEBUG_ASSERT(nextPacketSeqRel - nextSentAck <= static_cast<SeqNum>(AckBitset::size()));
				for (auto i = nextSentAck; i < nextPacketSeqRel; ++i) {
					auto& node = unacked[seqToIndex(i)];
					if (node) {
						// TODO: bandwidth
						const auto sz = static_cast<int32>(node->last - node->packet.head);
						sock.send(node->packet.head, sz, addr);
					}
				}
			}

			template<class C>
			void msgBegin(MessageType type, const C&) {
				static_assert((std::is_same_v<C, Cs> || ...), "Invalid network connection channel");
				// TODO: check that currNode is empty
				ENGINE_DEBUG_ASSERT(currNode == nullptr, "Attempting to begin new message without ending the previous message.");
				auto& node = getOrAllocPacket<C>();
				currNode = &node;

				write(MessageHeader{
					.type = type,
					.channel = getChannelId<C>(),
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
				ENGINE_DEBUG_ASSERT(sz <= MAX_MESSAGE_SIZE, "Message data exceeds MAX_MESSAGE_SIZE = ", MAX_MESSAGE_SIZE, " bytes.");

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
