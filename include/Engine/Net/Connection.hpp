#pragma once

// Meta
#include <Meta/IndexOf.hpp>

// Engine
#include <Engine/Clock.hpp>
#include <Engine/Net/BufferWriter.hpp>
#include <Engine/Net/net.hpp>
#include <Engine/Net/Packet.hpp>
#include <Engine/Net/UDPSocket.hpp>
#include <Engine/SequenceBuffer.hpp>


namespace Engine::Net {
	class MessageView {
		public:
			MessageHeader hdr = {};
			BufferReader msg = {};
	};

	template<class... Cs>
	class Connection {
		private:
			using ChannelId = uint8;

			const IPv4Address addr = {};
			uint16 keyLocal = 0;
			uint16 keyRemote = 0;

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

			// TODO: really this might be better as part of a subclass, nothing in here actually uses state other than debug checks.
			ConnectionState state = {};

			// TODO: channel - float32 sendBandwidth[sizeof...(Cs)] = {};
			// TODO: channel - float32 recvBandwidth[sizeof...(Cs)] = {};

			struct {
				/** The time the message was received */
				Engine::Clock::TimePoint time;

				/** The current byte in the recv packet */
				const byte* curr;

				/** One past the last byte in the recv packet */
				const byte* last;
			} rdat2;

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
				ENGINE_DEBUG_ASSERT(m <= maxMessageType(), "Invalid message type.");
				([&]<class T, T... Is>(std::integer_sequence<T, Is...>){
					using F = void(Func::*)(void) const;
					constexpr F fs[] = {&Func::template operator()<std::decay_t<decltype(std::declval<Connection>().getChannelForMessage<Is>())>>...};
					(func.*fs[m])();
				})(std::make_integer_sequence<MessageType, maxMessageType() + 1>{});
			}

		public:
			Connection(IPv4Address addr, Engine::Clock::TimePoint time) : addr{addr} {
				rdat2.time = time;
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

			ENGINE_INLINE void setKeyLocal(decltype(keyLocal) keyLocal) noexcept { this->keyLocal = keyLocal; }
			ENGINE_INLINE auto getKeyLocal() const noexcept { return keyLocal; }

			ENGINE_INLINE void setKeyRemote(decltype(keyRemote) keyRemote) noexcept { this->keyRemote = keyRemote; }
			ENGINE_INLINE auto getKeyRemote() const noexcept { return keyRemote; }

			constexpr static auto getChannelCount() noexcept { return sizeof...(Cs); }

			auto getAllChannelQueueSizes() {
				return std::array<int32, getChannelCount()>{getChannel<Cs>().getQueueSize()...};
			}

			ENGINE_INLINE ConnectionState getState() const noexcept { return state; }
			ENGINE_INLINE void setState(ConnectionState s) noexcept { state = s; }

			// TODO: why does this have a return value? isnt it always true?
			[[nodiscard]]
			bool recv(const Packet& pkt, int32 sz, Engine::Clock::TimePoint time) {
				rdat2.time = time;
				rdat2.curr = pkt.body;
				rdat2.last = pkt.head + sz;
				ENGINE_DEBUG_ASSERT(sz > 0);
				ENGINE_DEBUG_ASSERT(rdat2.curr < rdat2.last);

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

			auto recvTime() const { return rdat2.time; }

			MessageView recvNext() {
				ENGINE_DEBUG_ASSERT(rdat2.curr <= rdat2.last);

				MessageView view = {};

				// Read from channels if current packet empty
				if (rdat2.curr == rdat2.last) {
					// Its fine to use hdr directly here since channels store data in max aligned storage (std::vector at the time of writting this)
					// If we need more flexibility with storage we need to do a buffer reader like we do below.
					const MessageHeader* hdr = nullptr;
					((hdr = getChannel<Cs>().recvNext()) || ...); // Takes advantage of || short circuit
					if (hdr) { 
						view = {*hdr, {hdr, hdr->size + sizeof(MessageHeader)}};
						view.msg.read(sizeof(MessageHeader));
					}
					return view;
				}

				// Read from active packet
				view.msg = {rdat2.curr, rdat2.last};
				if (!view.msg.read(&view.hdr)) {
					ENGINE_DEBUG_ASSERT(false, "Unable to read message header.");
					ENGINE_WARN("Unable to read message header.");
					// TODO: we need to handle this or else we will just read the same message again
					return {};
				}

				ENGINE_DEBUG_ASSERT(view.msg.begin() != view.msg.peek());

				if (view.hdr.type > maxMessageType()) {
					ENGINE_WARN("Invalid message type.");
					ENGINE_DEBUG_ASSERT(false, "Invalid message type.");
					// TODO: we need to handle this or else we will just read the same message again
					return {};
				}

				if (view.hdr.size > sizeof(Packet::body) - sizeof(MessageHeader)) {
					ENGINE_WARN("Invalid message length");
					ENGINE_DEBUG_ASSERT(false, "Invalid message length");
					// TODO: we need to handle this or else we will just read the same message again
					return {};
				}

				// Retarget so we arent spanning the whole packet, only the active message.
				view.msg.resize(sizeof(MessageHeader) + view.hdr.size);

				// Should we process this message now or is it being buffered for later?
				bool process = true;
				callWithChannelForMessage(view.hdr.type, [&]<class C>(){
					auto& ch = getChannel<C>();
					process = ch.recv(view.hdr, std::as_const(view.msg));
				});

				rdat2.curr = view.msg.end();
				if (process) { return view; }
				return recvNext();
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
					msgBufferWriter.reset(pkt.body);
					(getChannel<Cs>().fill(seq, msgBufferWriter), ...);
					if (msgBufferWriter.size() == 0) { break; }
					++nextSeqNum;

					pkt.setKey(keyRemote); // TODO: should just be set once after packet is changed to member variable
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
			decltype(auto) beginMessage() {
				if constexpr (ENGINE_DEBUG) {
					if (!(state & getMessageMetaInfo<M>().sendState)) {
						ENGINE_WARN("Incorrect connection state to begin message of type ",
							getMessageMetaInfo<M>().name, "(", static_cast<size_t>(M), ")"
						);
					}
				}

				// TODO: check that no other message is active
				auto& channel = getChannelForMessage<M>();
				msgBufferWriter.reset(msgBuffer);

				// TODO: pass bufferwriter by ptr. we convert ot pointer anyways. makes it clearer
				return channel.beginMessage(channel, M, msgBufferWriter);
			}
	};
}
