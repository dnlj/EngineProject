#pragma once

// STD
#include <memory>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Net/MessageHeader.hpp>
#include <Engine/Net/IPv4Address.hpp>
#include <Engine/Net/UDPSocket.hpp>
#include <Engine/Net/Net.hpp>
#include <Engine/Net/PacketWriter.hpp>
#include <Engine/StaticVector.hpp>
#include <Engine/Bitset.hpp>
#include <Engine/Clock.hpp>
#include <Engine/Utility/Utility.hpp>


namespace Engine::Net {
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
			// TODO: static_assert that no two channels handle the same messages.
			// TODO: checks in recv to make sure that the message type is valid (handles by one of Cs)

			using NodePtr = std::unique_ptr<PacketNode>;

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

			PacketWriter packetWriter;

			template<class C>
			constexpr static ChannelId getChannelId() { return Meta::IndexOf<C, Cs...>::value; }

			
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

			///////////////////////////////////////////////////////////////////////////////////////////
			std::tuple<Cs...> channels;

			template<class C>
			C& getChannel() {
				return std::get<C>(channels);
			}

			template<auto M>
			auto& getChannelForMessage() {
				constexpr auto i = ((Cs::handlesMessageType<M>() ? getChannelId<Cs>() : 0) + ...);
				static_assert(i >= 0 && i <= std::tuple_size_v<decltype(channels)>);
				return std::get<i>(channels);
			}

			//template<class M>
			//constexpr static ChannelId getChannelIdForMessageType() {
			//	getChannelForMessageType<M>()
			//	return 0;
			//}


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
				// TODO:
				//if (pkt.getProtocol() != protocol) {
				//	ENGINE_WARN("Incorrect network protocol");
				//	return false;
				//}

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
							recvAcksUnrel <<= diff;
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
			/*void updateSentAcks(SeqNum first, AckBitset acks) {
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
			}*/

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
					while (auto node = packetWriter.pop()) {
						node->packet.setSeqNum(nextPacketSeqUnrel++);
						const auto seq = node->packet.getSeqNum();
						const auto i = seqToIndex(seq);
						auto& data = unrelAckData[i];

						// TODO: rm - debug
						if (data.recvTime == Engine::Clock::TimePoint{}) {
							ENGINE_WARN("NO ACK FOR ", seq - 64);
						}

						data = {
							.sendTime = now,
						};

						node->packet.setInitAck(lastRecvAckUnrel);
						node->packet.setAcks(recvAcksUnrel);

						const auto sz = static_cast<int32>(node->last - node->packet.head);
						sock.send(node->packet.head, sz, addr);
					}
				}
				
				/*{ // Reliable
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
				}*/
			}

			// TODO: should msgBegin/End be on packetwriter? somewhat makes sense since because we modify node->curr/last
			template<auto M>
			bool msgBegin() {
				if (!getChannelForMessage<M>().canWriteMessage()) { return false; }

				packetWriter.ensurePacketAvailable();

				write(MessageHeader{
					.type = M,
					.channel = 0, // TODO: impl
				}); 

				return true;
			}

			template<auto M>
			void msgEnd() {
				static_assert(sizeof(M) == sizeof(MessageHeader::type));

				auto node = packetWriter.back();
				ENGINE_DEBUG_ASSERT(node, "Unmatched Connection::msgEnd<", static_cast<int>(M), "> call.");

				auto* hdr = reinterpret_cast<MessageHeader*>(node->curr);
				ENGINE_DEBUG_ASSERT(hdr->type == M, "Mismatched msgBegin/End calls.");

				hdr->size = static_cast<decltype(hdr->size)>(node->size() - sizeof(*hdr));

				getChannelForMessage<M>().msgEnd(node->packet.getSeqNum(), *hdr);

				node->curr = node->last;
			}

			/**
			 * Writes data to the current message.
			 * @see PacketWriter::write
			 */
			template<class... Args>
			decltype(auto) write(Args&&... args) {
				return packetWriter.write(std::forward<Args>(args)...);
			}
	};
}
