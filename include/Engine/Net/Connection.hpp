#pragma once

// STD
#include <algorithm>
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
#include <Engine/SequenceBuffer.hpp>


namespace Engine::Net {
	// TODO: where to put this.
	struct ConnState {
		enum Type : uint8 {
			None         = 0,
			Disconnected  = 1 << 0,
			Connecting    = 1 << 1,
			Connected     = 1 << 2,
			Any           = Disconnected | Connecting | Connected,
		};
	};

	template<class... Cs>
	class Connection {
		private:
			using ChannelId = uint8;

			const IPv4Address addr = {};
			uint16 key = 0;

			struct PacketData {
				Engine::Clock::TimePoint sendTime;
				Engine::Clock::TimePoint recvTime;
			};

			SequenceBuffer<SeqNum, PacketData, AckBitset::size()> packetData;

			/** The next recv ack we are expecting */
			SeqNum nextRecvAck = {};

			/** Acks for the prev N packets before nextRecvAck */
			AckBitset recvAcks = {};

			constexpr static float64 pingSmoothing = 0.01; // TODO: ideal?
			Engine::Clock::Duration ping = {};

			
			constexpr static float32 lossSmoothing = 0.05f; // TODO: ideal?
			float loss = {};

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
				static_assert((Cs::handlesMessageType<M>() || ...), "No channel handles this message type.");
				static_assert((Cs::handlesMessageType<M>() + ...) == 1, "No two channels may handle the same message type.");

				constexpr auto i = ((Cs::handlesMessageType<M>() ? getChannelId<Cs>() : 0) + ...);
				static_assert(i >= 0 && i <= std::tuple_size_v<decltype(channels)>);

				return std::get<i>(channels);
			}

			template<class Func>
			void callWithChannelForMessage(const MessageType m, Func&& func) {
				([&]<class T, T... Is>(std::integer_sequence<T, Is...>){
					using F = void(Func::*)(void) const;
					constexpr F fs[] = {&Func::operator()<std::decay_t<decltype(std::declval<Connection>().getChannelForMessage<Is>())>>...};
					(func.*fs[m])();
				})(std::make_integer_sequence<MessageType, maxMessageType() + 1>{});
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

			auto getPing() const { return ping; }
			auto getLoss() const { return loss; }

			void setKey(decltype(key) key) { this->key = key; }
			auto getKey() const { return key; }

			[[nodiscard]]
			bool recv(const Packet& pkt, int32 sz, Engine::Clock::TimePoint time) {
				rdat.time = time;
				rdat.first = pkt.head;
				rdat.last = rdat.first + sz;
				rdat.curr = pkt.body;
				rdat.msgLast = rdat.curr;

				const auto& acks = pkt.getAcks();
				const auto seq = pkt.getSeqNum();

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

						// TODO: subtrackt 1/tickrate 
						ping += std::chrono::duration_cast<Engine::Clock::Duration>(
							((data->recvTime - data->sendTime) - ping) * pingSmoothing
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
				ENGINE_DEBUG_ASSERT(hdr->size <= MAX_MESSAGE_SIZE, "Invalid network message length");

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

				// TODO: this should probably be in its own function. Only fill empty space in packets. etc.
				(getChannel<Cs>().writeUnacked(packetWriter), ...);

				while (auto node = packetWriter.pop()) {
					const auto seq = node->packet.getSeqNum();
					{
						const float32 val = packetData.get(seq).recvTime == Engine::Clock::TimePoint{};
						loss += (val - loss) * lossSmoothing;
					}
					auto& data = packetData.insert(seq);

					data = {
						.sendTime = now,
					};

					node->packet.setKey(key);
					node->packet.setNextAck(nextRecvAck);
					node->packet.setAcks(recvAcks);
					
					const auto sz = static_cast<int32>(node->last - node->packet.head);
					sock.send(node->packet.head, sz, addr);
				}
			}

			// TODO: should msgBegin/End be on packetwriter? somewhat makes sense since because we modify node->curr/last
			template<auto M>
			bool msgBegin() {
				if (!getChannelForMessage<M>().canWriteMessage()) {
					__debugbreak(); // TODO: rm
					return false;
				}

				packetWriter.ensurePacketAvailable();

				write(MessageHeader{
					.type = M,
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

				// TODO: i dont think seqNum is actually set at this point. It is set in send?
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
