#pragma once

// Engine
#include <Engine/Net/MessageHeader.hpp>
#include <Engine/Net/PacketWriter.hpp>
#include <Engine/Clock.hpp>
#include <Engine/SequenceBuffer.hpp>


namespace Engine::Net {
	// TODO: make MAX_ACTIVE_MESSAGES_PER_CHANNEL a template argument of Channel_ReliableSender instead of global?
	constexpr inline int MAX_ACTIVE_MESSAGES_PER_CHANNEL = 64;

	template<class Channel>
	class MessageWriter {
		private:
			Channel& channel;
			PacketWriter* packetWriter = nullptr;
			MessageType type;

		public:
			MessageWriter(Channel& channel, PacketWriter* packetWriter, MessageType type)
				: channel{channel}
				, packetWriter{packetWriter}
				, type{type} {

				if (packetWriter) {
					packetWriter->ensurePacketAvailable();
					write(MessageHeader{
						.type = type,
					});
				}
			}

			MessageWriter(const MessageWriter&) = delete;

			~MessageWriter() {
				if (!*this) { return; }
				auto node = packetWriter->back();
				auto* hdr = reinterpret_cast<MessageHeader*>(node->curr);
				hdr->size = static_cast<decltype(hdr->size)>(node->size() - sizeof(*hdr));
				channel.msgEnd(node->packet.getSeqNum(), *hdr);
				packetWriter->advance();
			}

			ENGINE_INLINE operator bool() const noexcept { return packetWriter; }

			template<class... Args>
			ENGINE_INLINE void write(Args&&... args) {
				packetWriter->write(std::forward<Args>(args)...);
			}

			template<int N>
			ENGINE_INLINE void write(uint32 t) {
				packetWriter->write<N>(t);
			}

			ENGINE_INLINE void writeFlushBits() {
				packetWriter->writeFlushBits();
			}
	};

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

			// TODO: doc
			template<class Channel>
			[[nodiscard]]
			auto beginMessage(Channel& channel, PacketWriter* packetWriter, MessageType type) {
				return MessageWriter<Channel>(channel, packetWriter, type);
			}

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
			SequenceBuffer<SeqNum, MsgData, MAX_ACTIVE_MESSAGES_PER_CHANNEL> msgData; // TODO: ideal size?

			struct PacketData {
				std::vector<SeqNum> messages;
			};
			SequenceBuffer<SeqNum, PacketData, MAX_ACTIVE_MESSAGES_PER_CHANNEL> pktData; // TODO: ideal size?

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
						packetWriter.advance();

						//const auto TODO_rm = offsetof(MessageHeader, type);
						//const auto type = *static_cast<MessageType*>(msg->data.data() + TODO_rm);
						//ENGINE_LOG("MSG2: ", (int)type, " ", msg->data.size(), " ", seq, " ", packetWriter.getNextSeq() - 1);
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
			using Base = Channel_ReliableSender<Ms...>;
			// TODO: specialize for void data type
			SequenceBuffer<SeqNum, bool, MAX_ACTIVE_MESSAGES_PER_CHANNEL> recvData;

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
			using Base = Channel_ReliableSender<Ms...>;
			SeqNum nextRecvSeq = 0;

			struct RecvData {
				std::vector<byte> data;
			};
			SequenceBuffer<SeqNum, RecvData, MAX_ACTIVE_MESSAGES_PER_CHANNEL> recvData;

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

	constexpr int32 MAX_MESSAGE_BLOB_SIZE = 0x7FFFFFFF;

	template<class Channel>
	class MessageBlobWriter {
		private:
			Channel& channel;
			PacketWriter* packetWriter = nullptr;
			MessageType type;

		public:
			MessageBlobWriter(Channel& channel, PacketWriter* packetWriter, MessageType type)
				: channel{channel}
				, packetWriter{packetWriter}
				, type{type} {
			}

			~MessageBlobWriter() {
				// TODO: attempt to write this this message
			}

			ENGINE_INLINE operator bool() const noexcept { return packetWriter; }

			ENGINE_INLINE void writeBlob(const byte* data, int32 size) {
				ENGINE_DEBUG_ASSERT(size <= MAX_MESSAGE_BLOB_SIZE, "Attempting to send too much data");
				channel.writeBlob(*packetWriter, type, data, size);
			}
	};

	// TODO: update desc for Channel_LargeReliableOrdered
	/**
	 * A reliable ordered network channel.
	 * @see Channel_Base
	 */
	template<MessageType... Ms>
	class Channel_LargeReliableOrdered : public Channel_ReliableSender<Ms...> {
		private:
			using Base = Channel_ReliableSender<Ms...>;
			SeqNum nextRecvSeq = 0;
			SeqNum nextBlob = 0;

			SequenceBuffer<SeqNum, bool, MAX_ACTIVE_MESSAGES_PER_CHANNEL> recvData;

			struct WriteBlob {
				int32 curr = 0;
				std::vector<byte> data;
				MessageType type;
				const int32 remaining() const noexcept { return static_cast<int32>(data.size()) - curr; }
			};

			struct RecvBlob {
				int32 total = 0;
				std::vector<byte> data;

				ENGINE_INLINE bool complete() const noexcept {
					return total && (total == data.size() - sizeof(MessageHeader));
				}

				void insert(int32 i, const byte* start, const byte* stop) {
					i = i + sizeof(MessageHeader);
					const auto len = stop - start;
					if (static_cast<int32>(data.size()) < i + len) {
						data.resize(i + len);
					}
					ENGINE_LOG("Insert: ", len, " ", data.size());
					memcpy(&data[i], start, len);
				}
			};

			SequenceBuffer<SeqNum, WriteBlob, 8> writeBlobs;
			SequenceBuffer<SeqNum, RecvBlob, 8> recvBlobs;

			struct BlobHeader {
				constexpr static int32 LEN_MASK = 0x7FFFFFFF;
				// 4 bytes start
				// 2 bytes seqnum
				byte data[4 + 2];

				int32& start() { return *reinterpret_cast<int32*>(data); }
				const int32& start() const { return const_cast<BlobHeader*>(this)->start(); }

				SeqNum& seq() { return *reinterpret_cast<SeqNum*>(data + 4); }
				const SeqNum& seq() const { return const_cast<BlobHeader*>(this)->seq(); }
			};
			static_assert(sizeof(BlobHeader) == 4 + 2);

		public:
			bool canWriteMessage() const noexcept {
				return !writeBlobs.entryAt(nextBlob);
			};

			void writeBlob(PacketWriter& packetWriter, MessageType type, const byte* data, int32 size) {
				auto& blob = writeBlobs.insert(nextBlob);
				blob.type = type;
				blob.data.assign(data, data + size);
				attemptWriteBlob(packetWriter, nextBlob);
				++nextBlob;
			}

			bool recv(const MessageHeader& hdr) {
				if (recvData.canInsert(hdr.seq) && !recvData.contains(hdr.seq)) {
					recvData.insert(hdr.seq);

					const byte* dataBegin = reinterpret_cast<const byte*>(&hdr) + sizeof(hdr);

					const auto& info = *reinterpret_cast<const BlobHeader*>(dataBegin);
					dataBegin += sizeof(info);

					auto found = recvBlobs.find(info.seq());
					if (found == nullptr) {
						found = &recvBlobs.insert(info.seq());
						MessageHeader fake{
							.type = hdr.type,
						};
						found->insert(
							-static_cast<int32>(sizeof(fake)),
							reinterpret_cast<byte*>(&fake),
							reinterpret_cast<byte*>(&fake) + sizeof(fake)
						);
					}

					if (info.start() & ~BlobHeader::LEN_MASK) {
						found->total = *reinterpret_cast<const int32*>(dataBegin);
						dataBegin += sizeof(found->total);
					}

					const auto dataEnd = reinterpret_cast<const byte*>(&hdr) + sizeof(hdr) + hdr.size;

					// TODO: rm
					ENGINE_INFO("recv blob part: ", hdr.seq, " ", info.seq(), " ", dataEnd - dataBegin);

					found->insert(info.start() & BlobHeader::LEN_MASK, dataBegin, dataEnd);
					
					// TODO: rm
					if (found->complete()) {
						ENGINE_LOG("Blob ", info.seq(), " complete!");
					}
				}

				return false;
			}

			void attemptWriteBlob(PacketWriter& packetWriter, SeqNum seq) {
				auto* blob = writeBlobs.find(seq);
				if (!blob) { return; }
				
				packetWriter.ensurePacketAvailable();

				while (Base::canWriteMessage()) {
					if (blob->remaining() == 0) {
						writeBlobs.remove(seq);
						return;
					}

					const auto space = std::max(0,
						packetWriter.space()
						- static_cast<int32>(sizeof(MessageHeader))
						- static_cast<int32>(sizeof(BlobHeader))
						- static_cast<int32>(sizeof(int32)) // Optional size field
					);

					if (space < 32) { // Arbitrary minimum data size
						// TODO: actually want to adv packet yes?
						return;
					}

					auto msg = Base::beginMessage(*static_cast<Base*>(this), &packetWriter, blob->type);

					BlobHeader head;
					head.start() = blob->curr | (blob->curr > 0 ? 0 : ~BlobHeader::LEN_MASK);
					head.seq() = seq;
					msg.write(head);

					if (blob->curr == 0) {
						msg.write(static_cast<int32>(blob->data.size()));
					}

					const int32 len = std::min(space, blob->remaining());
					msg.write(blob->data.data() + blob->curr, len);
					blob->curr += len;
				}
			}

			void writeUnacked(PacketWriter& packetWriter) {
				for (auto seq = writeBlobs.minValid(); seqLess(seq, writeBlobs.max() + 1); ++seq) {
					attemptWriteBlob(packetWriter, seq);
				}

				Base::writeUnacked(packetWriter);
			}

			const MessageHeader* recvNext() {
				for (auto seq = recvBlobs.minValid(); seqLess(seq, recvBlobs.max() + 1); ++seq) {
					auto* blob = recvBlobs.find(seq);

					if (blob && blob->complete()) {
						recvBlobs.remove(seq);
						auto head = reinterpret_cast<MessageHeader*>(blob->data.data());
						head->size = blob->total;
						head->seq = seq;
						return head;
					}
				}

				return nullptr;
			}

			template<class Channel>
			[[nodiscard]]
			auto beginMessage(Channel& channel, PacketWriter* packetWriter, MessageType type) {
				return MessageBlobWriter<Channel>(channel, packetWriter, type);
			}
	};
}
