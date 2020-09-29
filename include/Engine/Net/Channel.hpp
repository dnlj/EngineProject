#pragma once

// Engine
#include <Engine/Net/MessageHeader.hpp>
#include <Engine/Net/PacketWriter.hpp>
#include <Engine/Clock.hpp>
#include <Engine/SequenceBuffer.hpp>


namespace Engine::Net {
	/**
	 * The base class used by all channels.
	 * @tparam Ms... A sequential list of messages handled by this channel.
	 */
	template<MessageType... Ms>
	class Channel_Base {
		private:
			static_assert(sizeof...(Ms) >= 1, "A channel must handle at least one message type.");

			constexpr static bool contiguous() noexcept {
				constexpr MessageType arr[] = {Ms...};
				for (int i = 1; i < sizeof...(Ms); ++i) {
					if (arr[i] != arr[i-1] + 1) { return false; }
				}
				return true;
			}
			static_assert(contiguous(), "The messages handled by a channel must be contiguous.");

		public:
			Channel_Base() = default;
			Channel_Base(const Channel_Base&) = delete;

			/**
			 * Get the maximum message type handled by this channel.
			 */
			constexpr static auto getMaxHandledMessageType() noexcept {
				// Doing `(0, Ms)` instead of just `Ms` is a work around for an internal compiler error.
				return ((0, Ms), ...);
			}

			/**
			 * Checks if a message is handled by this channel.
			 * @tparam M The message to check.
			 */
			template<auto M>
			constexpr static bool handlesMessageType() noexcept {
				return ((M == Ms) || ...);
			}

			/**
			 * Called the first time a connection receives an ack for a packet sequence number.
			 * Usually used for implementing message level reliability.
			 * @param seq The packet sequence number.
			 */
			constexpr static void recvPacketAck(SeqNum seq) noexcept {}

			/**
			 * Writes any messages that need (re)sending.
			 */
			constexpr static void writeUnacked(PacketWriter& packetWriter) noexcept {}

			/**
			 * Checks if this channel can have messages written to it.
			 */
			bool canWriteMessage() = delete;

			/**
			 * Determines if a message should be processed by a connection.
			 * @param hdr The header for the message.
			 * @return True if the messages should be processed.
			 */
			bool recv(const MessageHeader& hdr) = delete;

			/**
			 * Called once a message has been written.
			 * @param pktSeq The sequence number for the packet the message was written to.
			 * @param hdr The header for the message that was written.
			 */
			void msgEnd(SeqNum pktSeq, MessageHeader& hdr) = delete;

			/**
			 * Gets any messages that need to be processed from this channel.
			 * @return A pointer to the header of the message to process. If there is no message to process nullptr should be returned.
			 */
			constexpr static const MessageHeader* recvNext() noexcept { return nullptr; }
	};

	/**
	 * A unreliable unordered network channel.
	 * @see Channel_Base
	 */
	template<MessageType... Ms>
	class Channel_UnreliableUnordered : public Channel_Base<Ms...> {
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
	
	/**
	 * A unreliable ordered network channel.
	 * @see Channel_Base
	 */
	template<MessageType... Ms>
	class Channel_UnreliableOrdered : public Channel_Base<Ms...> {
		private:
			SeqNum nextSeq = -1;
			SeqNum lastSeq = -1;

		public:
			constexpr static bool canWriteMessage() noexcept {
				return true;
			}

			bool recv(const MessageHeader& hdr) noexcept {
				if (seqGreater(hdr.seq, lastSeq)) {
					lastSeq = hdr.seq;
					return true;
				}

				ENGINE_WARN("Old message"); // TODO: rm
				return false;
			}

			void msgEnd(SeqNum pktSeq, MessageHeader& hdr) {
				hdr.seq = ++nextSeq;
			}
	};
	
	/**
	 * Implements the sending portion of a reliable network channel.
	 * @see Channel_Base
	 */
	template<MessageType... Ms>
	class Channel_ReliableSender : public Channel_Base<Ms...> {
		protected:
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
			}

			void recvPacketAck(SeqNum pktSeq) {
				auto* pkt = pktData.find(pktSeq);
				if (!pkt) { return; }

				for (SeqNum s : pkt->messages) {
					if (msgData.find(s)) {
						msgData.remove(s);
					}
				}

				pktData.remove(pktSeq);
			}

			// TODO: also want some kind of fill-rest-of-packet function

			void writeUnacked(PacketWriter& packetWriter) {
				const auto now = Engine::Clock::now();
				// BUG: at our current call rate this may overwrite packets before we have a chance to ack them because they are overwritten with a new packets info
				for (auto seq = msgData.minValid(); seqLess(seq, msgData.max() + 1); ++seq) {
					auto* msg = msgData.find(seq);

					// TODO: resend time should be configurable per channel
					if (msg && (now > msg->lastSendTime + std::chrono::milliseconds{50})) {
						msg->lastSendTime = now;
						packetWriter.ensurePacketAvailable(); // TODO: seems kinda hacky
						packetWriter.write(msg->data.data(), msg->data.size());

						const auto TODO_rm = offsetof(MessageHeader, type);
						const auto type = *static_cast<MessageType*>(msg->data.data() + TODO_rm);
						ENGINE_LOG("MSG2: ", (int)type, " ", msg->data.size(), " ", seq, " ", packetWriter.getNextSeq() - 1);
						addMessageToPacket(packetWriter.getNextSeq() - 1, seq);
					}
				}
			}
	};
	
	/**
	 * A reliable unordered network channel.
	 * @see Channel_Base
	 */
	template<MessageType... Ms>
	class Channel_ReliableUnordered : public Channel_ReliableSender<Ms...> {
		private:
			// TODO: specialize for void data type
			SequenceBuffer<SeqNum, bool, decltype(msgData)::capacity()> recvData;

		public:
			bool recv(const MessageHeader& hdr) {
				if (recvData.canInsert(hdr.seq) && !recvData.contains(hdr.seq)) {
					recvData.insert(hdr.seq);
					return true;
				}

				return false;
			}
	};
	
	/**
	 * A reliable ordered network channel.
	 * @see Channel_Base
	 */
	template<MessageType... Ms>
	class Channel_ReliableOrdered : public Channel_ReliableSender<Ms...> {
		private:
			SeqNum nextRecvSeq = 0;

			struct RecvData {
				std::vector<byte> data;
			};
			SequenceBuffer<SeqNum, RecvData, decltype(msgData)::capacity()> recvData;

		public:
			bool recv(const MessageHeader& hdr) {
				if (recvData.canInsert(hdr.seq) && !recvData.contains(hdr.seq)) {
					auto& rcv = recvData.insert(hdr.seq);
					rcv.data.assign(reinterpret_cast<const byte*>(&hdr), reinterpret_cast<const byte*>(&hdr) + sizeof(hdr) + hdr.size);
				}
				return false;
			}

			const MessageHeader* recvNext() {
				auto* found = recvData.find(nextRecvSeq);

				if (found) {
					++nextRecvSeq;
					return reinterpret_cast<MessageHeader*>(found->data.data());
				}

				return nullptr;
			}
	};
}
