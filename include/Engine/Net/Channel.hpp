#pragma once

// Engine
#include <Engine/Net/MessageHeader.hpp>
#include <Engine/Net/PacketWriter.hpp>
#include <Engine/Clock.hpp>
#include <Engine/SequenceBuffer.hpp>


namespace Engine::Net::Detail {
	template<MessageType... Ms>
	class Channel_Base { // TODO: name?
		// TODO: static_assert that all messages ids are continuous
		public:
			Channel_Base() = default;
			Channel_Base(const Channel_Base&) = delete;

			template<auto M>
			constexpr static bool handlesMessageType() {
				return ((M == Ms) || ...);
			}

			void recvPacketAck(SeqNum seq) {}
			void writeUnacked(PacketWriter& packetWriter) {}

	};
}

namespace Engine::Net {
	template<MessageType... Ms>
	class Channel_UnreliableUnordered : public Detail::Channel_Base<Ms...> { // TODO: name?
		private:
			SeqNum nextSeq = -1;

		public:
			constexpr static bool canWriteMessage() noexcept {
				return true;
			}

			constexpr static bool recv(const MessageHeader& hdr) noexcept {
				return true;
			}

			void msgEnd(SeqNum pktSeq, MessageHeader& hdr) {
				hdr.seq = ++nextSeq;
			}
	};

	template<MessageType... Ms>
	class Channel_ReliableUnordered : public Detail::Channel_Base<Ms...> { // TODO: name?
		private:
			/** The sequence number to use for the next message */
			SeqNum nextSeq = 0;

			struct MsgData {
				Engine::Clock::TimePoint lastSendTime;
				std::vector<byte> data;
			};
			SequenceBuffer<SeqNum, MsgData, 64> msgData; // TODO: ideal size?

			struct PacketData {
				std::vector<SeqNum> messages;
			};
			SequenceBuffer<SeqNum, PacketData, 64> pktData; // TODO: ideal size?

			// TODO: specialize for void data type
			SequenceBuffer<SeqNum, bool, decltype(msgData)::capacity()> recvData;

			void addMessageToPacket(SeqNum pktSeq, SeqNum msgSeq) {
				auto* pkt = pktData.find(pktSeq);
				if (!pkt) { pkt = &pktData.insert(pktSeq); }
				pkt->messages.push_back(msgSeq);
			}

		public:
			bool canWriteMessage() const {
				return !msgData.entryAt(nextSeq);
			}

			void msgEnd(SeqNum pktSeq, MessageHeader& hdr) {
				hdr.seq = nextSeq++;

				ENGINE_DEBUG_ASSERT(msgData.canInsert(hdr.seq));
				auto& msg = msgData.insert(hdr.seq);
				msg.data.assign(reinterpret_cast<byte*>(&hdr), reinterpret_cast<byte*>(&hdr) + sizeof(hdr) + hdr.size);
				msg.lastSendTime = Engine::Clock::now();

				addMessageToPacket(pktSeq, hdr.seq);

				ENGINE_LOG("RU - msgEnd: ", pktSeq, " ", hdr.seq);
			}

			void recvPacketAck(SeqNum pktSeq) {
				auto* pkt = pktData.find(pktSeq);
				if (!pkt) { return; }

				for (SeqNum s : pkt->messages) {
					if (msgData.find(s)) {
						msgData.remove(s);
						ENGINE_LOG("RU - ACK: ", pktSeq, " ", s, " ", msgData.minValid(), " ", msgData.max());
					} else {
						ENGINE_LOG("RU - DUP: ", pktSeq, " ", s, " ", msgData.minValid(), " ", msgData.max());
					}
				}

				pktData.remove(pktSeq);
			}

			
			bool recv(const MessageHeader& hdr) {
				if (recvData.canInsert(hdr.seq) && !recvData.contains(hdr.seq)) {
					recvData.insert(hdr.seq);
					// TODO: rm ENGINE_LOG("RU - recv: ack ", hdr.seq);
					return true;
				}

				// TODO: rm ENGINE_LOG("RU - recv: dup ", hdr.seq);

				return false;
			}

			// TODO: also want some kind of fill-rest-of-packet function

			void writeUnacked(PacketWriter& packetWriter) {
				const auto now = Engine::Clock::now();
				// TODO: BUG: at our current call rate this may overwrite packets before we have a chance to ack them because they are overwritten with a new packets info
				for (auto seq = msgData.minValid(); seqLess(seq, msgData.max() + 1); ++seq) {
					auto* msg = msgData.find(seq);

					// TODO: resend time should be configurable per channel
					if (msg && (now > msg->lastSendTime + std::chrono::milliseconds{50})) {
						msg->lastSendTime = now;
						packetWriter.ensurePacketAvailable(); // TODO: seems kinda hacky
						packetWriter.write(msg->data.data(), msg->data.size());
						ENGINE_LOG("RU - resend: ", (packetWriter.getNextSeq() - 1), " ", seq, " ", now.time_since_epoch().count());
						addMessageToPacket(packetWriter.getNextSeq() - 1, seq);
					}
				}
			}
	};
}
