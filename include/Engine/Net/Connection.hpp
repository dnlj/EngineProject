#pragma once

// STD
#include <algorithm>
#include <memory>

// Meta
#include <Meta/IndexOf.hpp>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Net/MessageHeader.hpp>
#include <Engine/Net/IPv4Address.hpp>
#include <Engine/Net/UDPSocket.hpp>
#include <Engine/Net/Net.hpp>
#include <Engine/Net/BufferWriter.hpp>
#include <Engine/StaticVector.hpp>
#include <Engine/Bitset.hpp>
#include <Engine/Clock.hpp>
#include <Engine/Utility/Utility.hpp>
#include <Engine/SequenceBuffer.hpp>
#include <Engine/Net/Packet.hpp>


namespace Engine::Net {
	template<class State, class... Cs>
	class Connection {
		private:
			using ChannelId = uint8;

			const IPv4Address addr = {};
			uint16 keySend = 0;
			uint16 keyRecv = 0;

			struct PacketData {
				Engine::Clock::TimePoint sendTime;
				Engine::Clock::TimePoint recvTime;
			};

			SequenceBuffer<SeqNum, PacketData, AckBitset::size()> packetData;
			byte msgBuffer[sizeof(Packet::body)];
			BufferWriter msgBufferWriter;

			/** How many packets per second we can send */
			float32 packetSendRate = 16.0f;

			/** How many packets per second we can recv. Not used locally. Should be networked to server. */
			float32 packetRecvRate = 32.0f;

			/** Current maximum number of packet we can send */
			float32 packetSendBudget = 0;

			/** Last time we updated the packet budget */
			Clock::TimePoint lastBudgetUpdate = {};

			/** The next recv ack we are expecting */
			SeqNum nextRecvAck = {};

			/** Acks for the prev N packets before nextRecvAck */
			AckBitset recvAcks = {};

			constexpr static float64 pingSmoothing = 0.02;
			Engine::Clock::Duration ping = std::chrono::milliseconds{50};
			
			constexpr static float64 jitterSmoothing = 0.02;
			Engine::Clock::Duration jitter = {};

			constexpr static float32 lossSmoothing = 0.01f;
			float32 loss = {};

			constexpr static float32 bandwidthSmoothing = 0.01f;
			Engine::Clock::TimePoint lastBandwidthUpdate = {};
			float32 packetSendBandwidth = 0;
			float32 packetSentBandwidthAccum = 0;
			float32 packetRecvBandwidth = 0;
			float32 packetRecvBandwidthAccum = 0;
			uint32 packetTotalBytesSent = 0;
			uint32 packetTotalBytesRecv = 0;

			uint64 bitStore = 0;
			int bitCount = 0;

			State state = {};


			// TODO: channel - float32 sendBandwidth[sizeof...(Cs)] = {};
			// TODO: channel - float32 recvBandwidth[sizeof...(Cs)] = {};

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

			SeqNum nextSeqNum = 0;

			template<class C>
			constexpr static ChannelId getChannelId() { return ::Meta::IndexOf<C, Cs...>::value; }

			constexpr static auto maxMessageType() noexcept {
				return std::max({Cs::getMaxHandledMessageType() ...});
			}

			///////////////////////////////////////////////////////////////////////////////////////////
			std::tuple<Cs...> channels;

			template<class C>
			C& getChannel() {
				return std::get<C>(channels);
			}

			template<auto M>
			auto& getChannelForMessage() {
				static_assert(M <= maxMessageType(), "Invalid message type.");
				static_assert((Cs::template handlesMessageType<M>() || ...), "No channel handles this message type.");
				static_assert((Cs::template handlesMessageType<M>() + ...) == 1, "No two channels may handle the same message type.");

				constexpr auto i = ((Cs::template handlesMessageType<M>() ? getChannelId<Cs>() : 0) + ...);
				static_assert(i >= 0 && i <= std::tuple_size_v<decltype(channels)>);

				return std::get<i>(channels);
			}

			template<class Func>
			void callWithChannelForMessage(const MessageType m, Func&& func) {
				([&]<class T, T... Is>(std::integer_sequence<T, Is...>){
					using F = void(Func::*)(void) const;
					constexpr F fs[] = {&Func::template operator()<std::decay_t<decltype(std::declval<Connection>().getChannelForMessage<Is>())>>...};
					(func.*fs[m])();
				})(std::make_integer_sequence<MessageType, maxMessageType() + 1>{});
			}

		public:
			Connection(IPv4Address addr, Engine::Clock::TimePoint time) : addr{addr} {
				rdat.time = time;
			}

			ENGINE_INLINE const auto& address() const { return addr; }

			ENGINE_INLINE auto getPacketSendBudget() const noexcept { return packetSendBudget; }

			ENGINE_INLINE auto getPacketSendRate() const noexcept { return packetSendRate; }
			ENGINE_INLINE auto setPacketSendRate(float32 r) noexcept { packetSendRate = r; }

			ENGINE_INLINE auto getPacketRecvRate() const noexcept { return packetRecvRate; }
			ENGINE_INLINE auto setPacketRecvRate(float32 r) noexcept { packetRecvRate = r; }

			ENGINE_INLINE auto getPing() const noexcept { return ping; }
			ENGINE_INLINE auto getLoss() const noexcept { return loss; }
			ENGINE_INLINE auto getJitter() const noexcept { return jitter; }
			ENGINE_INLINE auto getSendBandwidth() const noexcept { return packetSendBandwidth; }
			ENGINE_INLINE auto getRecvBandwidth() const noexcept { return packetRecvBandwidth; }
			ENGINE_INLINE auto getTotalBytesSent() const noexcept { return packetTotalBytesSent; }
			ENGINE_INLINE auto getTotalBytesRecv() const noexcept { return packetTotalBytesRecv; }

			ENGINE_INLINE void setKeySend(decltype(keySend) keySend) noexcept { this->keySend = keySend; }
			ENGINE_INLINE auto getKeySend() const noexcept { return keySend; }

			ENGINE_INLINE void setKeyRecv(decltype(keyRecv) keyRecv) noexcept { this->keyRecv = keyRecv; }
			ENGINE_INLINE auto getKeyRecv() const noexcept { return keyRecv; }

			constexpr static auto getChannelCount() noexcept { return sizeof...(Cs); }

			int32 getChannelQueueSize(int32 c) {
				int32 sizes[] = {getChannel<Cs>().getQueueSize()...};
				return (c < std::size(sizes)) ? sizes[c] : -1;
			}

			auto getAllChannelQueueSizes() {
				return std::array<int32, getChannelCount()>{getChannel<Cs>().getQueueSize()...};
			}

			ENGINE_INLINE State getState() const noexcept { return state; }
			ENGINE_INLINE void setState(State s) noexcept { state = s; }

			// TODO: why does this have a return value? isnt it always true?
			[[nodiscard]]
			bool recv(const Packet& pkt, int32 sz, Engine::Clock::TimePoint time) {
				rdat.time = time;
				rdat.first = pkt.head;
				rdat.last = rdat.first + sz;
				rdat.curr = pkt.body;
				rdat.msgLast = rdat.curr;

				const auto& acks = pkt.getAcks();
				const auto seq = pkt.getSeqNum();
				packetRecvBandwidthAccum += sz;

				// Update recv packet info
				do { 
					const auto min = nextRecvAck - acks.size();
					if (seqLess(seq, min))  { continue; }

					while (seqLess(nextRecvAck, seq + 1)) {
						recvAcks.reset(++nextRecvAck  % recvAcks.size());
					}

					recvAcks.set(seq % recvAcks.size());
				} while(0);

				// Update sent packet info
				{
					const auto& next = pkt.getNextAck();
					const auto min = next - AckBitset::size();
					for (auto s = min; seqLess(s, next); ++s) {
						if (!acks.test(s % acks.size())) { continue; }

						auto* data = packetData.find(s);
						if (!data || data->recvTime != Engine::Clock::TimePoint{}) { continue; }

						data->recvTime = time;
						
						const auto pktPing = data->recvTime - data->sendTime;
						
						jitter += std::chrono::duration_cast<Engine::Clock::Duration>(
							(std::chrono::abs(pktPing - ping) - jitter) * jitterSmoothing
						);

						// TODO: subtrackt 1/tickrate 
						ping += std::chrono::duration_cast<Engine::Clock::Duration>(
							(pktPing - ping) * pingSmoothing
						);

						(getChannel<Cs>().recvPacketAck(s), ...);
					}
				}

				return true;
			}

			auto recvTime() const { return rdat.time; }

			/**
			 * Read the next message from the packet set by recv.
			 */
			const MessageHeader* recvNext() {
				if (rdat.curr >= rdat.last) {
					const MessageHeader* hdr = nullptr;
					((hdr = getChannel<Cs>().recvNext()) || ...); // Takes advantage of || short circuit
					if (hdr) {
						rdat.first = reinterpret_cast<const byte*>(hdr);
						rdat.curr = rdat.first + sizeof(*hdr);
						rdat.last = rdat.curr + hdr->size;
						rdat.msgLast = rdat.last;
					}
					return hdr;
				}

				ENGINE_DEBUG_ASSERT(rdat.curr == rdat.msgLast, "Incomplete read of network message");
				const auto* hdr = read<MessageHeader>();
				ENGINE_DEBUG_ASSERT(hdr->size <= sizeof(Packet::body) - sizeof(MessageHeader),
					"Invalid network message length"
				);

				bool process = true;
				callWithChannelForMessage(hdr->type, [&]<class C>(){
					auto& ch = getChannel<C>();
					process = ch.recv(*hdr);
				});

				rdat.msgLast = rdat.curr + hdr->size;

				if (process) {
					return hdr;
				} else {
					rdat.curr += hdr->size;
					return recvNext();
				}
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

				// TODO: will this have alignment issues?
				const void* temp = rdat.curr;
				rdat.curr += sz;
				return temp;
			}
			
			/**
			 * Reads an object from the current message.
			 */
			template<class T>
			decltype(auto) read() {
				static_assert(!std::is_pointer_v<T>, "Cannot read pointers from network connection.");
				ENGINE_DEBUG_ASSERT(bitCount == 0, "Incomplete read of packed bits");
				return reinterpret_cast<const T*>(read(sizeof(T)));
			}

			template<int N>
			uint32 read() {
				// TODO: add version for > 32
				static_assert(N <= 32);
				while (bitCount < N) {
					bitStore |= uint64{*rdat.curr} << bitCount;
					rdat.curr += 1;
					bitCount += 8;
				}

				uint32 val = bitStore & ((1ull << N) - 1);
				bitStore >>= N;
				bitCount -= N;
				return val;
			}

			void readFlushBits() {
				bitStore = 0;
				bitCount = 0;
			}

			void send(UDPSocket& sock) {
				const auto now = Engine::Clock::now();

				// Update packet budget
				packetSendBudget += Clock::Seconds{now - lastBudgetUpdate}.count() * packetSendRate;
				packetSendBudget = std::min(packetSendBudget, packetSendRate);
				lastBudgetUpdate = now;

				// Write + send packets
				while (packetSendBudget >= 1) {
					const auto seq = nextSeqNum;

					Packet pkt; // TODO: if we keep this move to be a member variable instead; - should be able to merge with msgBuffer?
					msgBufferWriter = pkt.body;
					(getChannel<Cs>().fill(seq, msgBufferWriter), ...);
					if (msgBufferWriter.size() == 0) { break; }
					++nextSeqNum;

					pkt.setKey(keySend); // TODO: should just be set once after packet is changed to member variable
					pkt.setNextAck(nextRecvAck);
					pkt.setAcks(recvAcks);
					pkt.setProtocol(protocol); // TODO: should just be set once after packet is changed to member variable
					pkt.setSeqNum(seq);

					{
						const float32 val = packetData.get(seq).recvTime == Engine::Clock::TimePoint{};
						loss += (val - loss) * lossSmoothing;
					}

					packetData.insert(seq) = { .sendTime = now, };

					const auto sz = sizeof(pkt.head) + msgBufferWriter.size();
					packetSentBandwidthAccum += sz;
					sock.send(&pkt, (int32)sz, addr);
					packetSendBudget -= 1;
				}

				const auto diff = now - lastBandwidthUpdate;
				lastBandwidthUpdate = now;
				Engine::Clock::Seconds sec = diff;
				packetSendBandwidth += (packetSentBandwidthAccum / sec.count() - packetSendBandwidth) * bandwidthSmoothing;
				packetRecvBandwidth += (packetRecvBandwidthAccum / sec.count() - packetRecvBandwidth) * bandwidthSmoothing;
				packetTotalBytesSent += static_cast<int32>(packetSentBandwidthAccum);
				packetTotalBytesRecv += static_cast<int32>(packetRecvBandwidthAccum);
				packetRecvBandwidthAccum = 0;
				packetSentBandwidthAccum = 0;
			}

			template<auto M>
			[[nodiscard]]
			ENGINE_INLINE decltype(auto) beginMessage() {
				// TODO: check that no other message is active
				auto& channel = getChannelForMessage<M>();
				using Writer = decltype(channel.beginMessage(channel, M, msgBufferWriter));

				if constexpr (ENGINE_DEBUG) {
					if (!(state & MessageTraits<M>::state)) {
						ENGINE_WARN("Incorrect connection state to begin message of type ",
							MessageTraits<M>::name, "(", static_cast<size_t>(M), ")"
						);
					}
				}

				msgBufferWriter = msgBuffer;

				// TODO: pass bufferwriter by ptr. we convert ot pointer anyways. makes it clearer
				return channel.beginMessage(channel, M, msgBufferWriter);
			}
	};
}
