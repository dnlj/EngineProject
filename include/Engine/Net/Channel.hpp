#pragma once

// Engine
#include <Engine/Net/MessageHeader.hpp>
#include <Engine/Net/BufferWriter.hpp>
#include <Engine/Net/Packet.hpp> // TODO: once we have configurable limits this isnt needed
#include <Engine/Clock.hpp>
#include <Engine/SequenceBuffer.hpp>

// TODO (0W4vlcPN): We shouldn't be calling SequenceBuffer::insert anywhere in
//                  the channels since it leads to a lot of potentially large
//                  and redundant allocations. The whole point was to reuse the
//                  same buffer data instead of reallocating every time. I think
//                  this is fallout from reusing SequenceBuffer in other non
//                  network code later and then assuming insert would re-init
//                  its data. We should either split into another class or
//                  manually call insertNoInit and clear/reuse the fields
//                  appropriately.


namespace Engine::Net {
	// TODO: make MAX_ACTIVE_MESSAGES_PER_CHANNEL a template argument of Channel_ReliableSender instead of global?
	constexpr inline int MAX_ACTIVE_MESSAGES_PER_CHANNEL = 64;

	template<class Channel>
	class MessageWriter {
		private:
			BufferWriter* buff;
			Channel& channel;

		public:
			MessageWriter(Channel& channel, MessageType type, BufferWriter* buff)
				: buff{buff} 
				, channel{channel} {

				if (buff) {
					ENGINE_DEBUG_ASSERT(buff->size() == 0, "Non-empty buffer given to MessageWriter.");
					buff->write(MessageHeader{
						.type = type,
					});
				}
			}

			~MessageWriter() {
				if (buff) {
					auto* hdr = reinterpret_cast<MessageHeader*>(buff->data());
					hdr->size = static_cast<uint16>(buff->size() - sizeof(MessageHeader));
					channel.endMessage(*buff);
				}
			}

			ENGINE_INLINE operator bool() const noexcept {
				return buff;
			}

			ENGINE_INLINE BufferWriter& getBufferWriter() noexcept {
				ENGINE_DEBUG_ASSERT(buff);
				return *buff;
			}

			template<class... Args>
			ENGINE_INLINE decltype(auto) write(Args&&... args) {
				return buff->write(std::forward<Args>(args)...);
			}

			template<int N>
			ENGINE_INLINE void write(uint32 t) {
				return buff->write<N>(t);
			}

			ENGINE_INLINE void writeFlushBits() {
				buff->writeFlushBits();
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
			 * Checks if this channel can have messages written to it.
			 */
			bool canWriteMessage() = delete;

			/**
			 * Returns a new message object used to write to this channel.
			 * @param channel The channel on which this is being called.
			 * @param type The type of the message. @see MessageHeader
			 * @param buff The buffer writer used to build the message.
			 */
			template<class Channel>
			[[nodiscard]]
			auto beginMessage(Channel& channel, MessageType type, BufferWriter& buff) {
				return MessageWriter<Channel>{channel, type, channel.canWriteMessage() ? &buff : nullptr};
			}

			/**
			 * Called once a message returned by beginMessage is complete.
			 * @param buff The buffer the message was written to.
			 */
			void endMessage(BufferWriter& buff) = delete;

			/**
			 * Fills the given packet/buffer with messages from this channel.
			 * @param pktSeq The sequence number of the packet to fill.
			 * @param buff The buffer to write the messages to.
			 */
			void fill(SeqNum pktSeq, BufferWriter& buff) = delete;

			/**
			 * Determines if a message should be processed by a connection.
			 * @param hdr The header for the message.
			 * @param buff A view of the message with the current position set to the body of the message.
			 * @return True if the messages should be processed.
			 */
			bool recv(const MessageHeader& hdr, BufferReader buff) = delete;

			/**
			 * Gets any messages that need to be processed from this channel.
			 * @return A pointer to the header of the message to process. If there is no message to process nullptr should be returned.
			 */
			constexpr static const MessageHeader* recvNext() noexcept { return nullptr; }

			/**
			 * Gets the number of messages waiting to be sent.
			 */
			int32 getQueueSize() = delete;
	};

	/**
	 * A unreliable unordered network channel.
	 * @see Channel_Base
	 */
	template<MessageType... Ms>
	class Channel_UnreliableUnordered : public Channel_Base<Ms...> {
		private:
			SeqNum nextSeq = 0;
			std::vector<std::vector<byte>> messages;

		public:
			constexpr static bool canWriteMessage() noexcept {
				return true;
			}

			constexpr static bool recv(const MessageHeader& hdr, BufferReader buff) noexcept {
				return true;
			}
			
			void endMessage(BufferWriter& buff) {
				auto* hdr = reinterpret_cast<MessageHeader*>(buff.data());
				hdr->seq = nextSeq++;
				messages.emplace_back(buff.cbegin(), buff.cend());
			}
			
			void fill(SeqNum pktSeq, BufferWriter& buff) {
				while (messages.size()) {
					const auto& msg = messages.back();
					if (buff.write(msg.data(), msg.size())) {
						messages.pop_back();
					} else {
						break;
					}
				}

				messages.clear();
			}

			int32 getQueueSize() const noexcept {
				return static_cast<int32>(messages.size());
			}
	};
	
	/**
	 * A unreliable ordered network channel.
	 * @see Channel_Base
	 */
	template<MessageType... Ms>
	class Channel_UnreliableOrdered : public Channel_Base<Ms...> {
		private:
			SeqNum nextSeq = 0;
			SeqNum lastSeq = -1;
			std::vector<std::vector<byte>> messages;

		public:
			constexpr static bool canWriteMessage() noexcept {
				return true;
			}

			bool recv(const MessageHeader& hdr, BufferReader buff) noexcept {
				if (Math::Seq::greater(hdr.seq, lastSeq)) {
					lastSeq = hdr.seq;
					return true;
				}

				return false;
			}

			void endMessage(BufferWriter& buff) {
				auto* hdr = reinterpret_cast<MessageHeader*>(buff.data());
				hdr->seq = nextSeq++;
				messages.emplace_back(buff.cbegin(), buff.cend());
			}
			
			void fill(SeqNum pktSeq, BufferWriter& buff) {
				while (messages.size()) {
					const auto& msg = messages.back();
					if (buff.write(msg.data(), msg.size())) {
						messages.pop_back();
					} else {
						break;
					}
				}

				messages.clear();
			}

			int32 getQueueSize() const noexcept {
				return messages.size();
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
			int32 getQueueSize() const noexcept {
				return msgData.span();
			}

			bool canWriteMessage() const {
				return !msgData.entryAt(nextSeq);
			}

			void endMessage(BufferWriter& buff) {
				ENGINE_DEBUG_ASSERT(canWriteMessage());
				auto* hdr = reinterpret_cast<MessageHeader*>(buff.data());
				hdr->seq = nextSeq++;

				ENGINE_DEBUG_ASSERT(msgData.canInsert(hdr->seq));
				auto& msg = msgData.insert(hdr->seq);
				msg.data.assign(buff.cbegin(), buff.cend());
				msg.lastSendTime = {};
			}
			
			void fill(SeqNum pktSeq, BufferWriter& buff) {
				const auto now = Engine::Clock::now();
				// BUG: at our current call rate this may overwrite packets before we have a chance to ack them because they are overwritten with a new packets info
				for (auto seq = msgData.minValid(); Math::Seq::less(seq, msgData.max() + 1); ++seq) {
					auto* msg = msgData.find(seq);

					// TODO: resend time should be configurable per channel
					if (msg && (now > msg->lastSendTime + std::chrono::milliseconds{50})) {
						if (buff.write(msg->data.data(), msg->data.size())) {
							msg->lastSendTime = now;
							addMessageToPacket(pktSeq, seq);
						}
					}
				}
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
			bool recv(const MessageHeader& hdr, BufferReader buff) {
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
			bool recv(const MessageHeader& hdr, const BufferReader buff) {
				if (recvData.canInsert(hdr.seq) && !recvData.contains(hdr.seq)) {
					auto& rcv = recvData.insert(hdr.seq);
					rcv.data.assign(buff.begin(), buff.end());
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
			MessageType type;
			BufferWriter* buff = nullptr;

		public:
			MessageBlobWriter(Channel& channel, MessageType type, BufferWriter* buff)
				: channel{channel}
				, type{type} 
				, buff{buff} {
			}

			~MessageBlobWriter() {
			}

			ENGINE_INLINE operator bool() const noexcept { return buff; }

			ENGINE_INLINE void writeBlob(const byte* data, int32 size) {
				ENGINE_DEBUG_ASSERT(size <= MAX_MESSAGE_BLOB_SIZE, "Attempting to send too much data");
				channel.writeBlob(*buff, type, data, size);
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

			struct Range {
				int32 start;
				int32 stop;

				ENGINE_INLINE int32 size() const noexcept { return stop - start; }
				ENGINE_INLINE bool operator<(const Range& other) const noexcept {
					return start < other.start;
				}
			};

			struct WriteBlob {
				int32 curr = 0;
				std::vector<byte> data;
				MessageType type;
				SeqNum parts;
				const int32 remaining() const noexcept { return static_cast<int32>(data.size()) - curr; }
			};

			struct RecvBlob {
				int32 total = 0;
				std::vector<byte> data;
				std::vector<Range> parts;

				ENGINE_INLINE bool complete() const noexcept {
					return (parts.size() == 1) && (parts.begin()->size() == total + sizeof(MessageHeader));
					//return total && (total == data.size() - sizeof(MessageHeader));
				}

				void insert(int32 i, const byte* start, const byte* stop) {
					i = i + sizeof(MessageHeader);
					const int32 len = static_cast<int32>(stop - start);
					if (static_cast<int32>(data.size()) < i + len) {
						data.resize(i + len);
					}
					// ENGINE_LOG("Insert: ", len, " ", data.size());
					memcpy(&data[i], start, len);

					const Range range {i, i + len};
					auto found = std::upper_bound(parts.begin(), parts.end(), range);

					// Merge with after
					if (found != parts.cend() && range.stop == found->start) {
						// ENGINE_LOG("Merge after: (", range.start, ", ", range.stop, ") U (", found->start, ", ", found->stop, ")");
						found->start = range.start;
					} else {
						found = parts.insert(found, range);
					}

					// Merge with before
					if (found > parts.cbegin()) {
						auto prev = found - 1;
						if (prev->stop == found->start) {
							// ENGINE_LOG("Merge before: (", prev->start, ", ", prev->stop, ") U (", found->start, ", ", found->stop, ")");
							prev->stop = found->stop;
							parts.erase(found);
							found = prev;
						}
					}
				}
			};

			constexpr static int MAX_BLOBS = 8; // TODO: this should be configurable
			SequenceBuffer<SeqNum, WriteBlob, MAX_BLOBS> writeBlobs;
			SequenceBuffer<SeqNum, RecvBlob, MAX_BLOBS> recvBlobs;

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

			template<class Channel>
			[[nodiscard]]
			auto beginMessage(Channel& channel, MessageType type, BufferWriter& buff) {
				return MessageBlobWriter<Channel>{channel, type, canWriteMessage() ? &buff : nullptr};
			}

			void writeBlob(BufferWriter& buff, MessageType type, const byte* data, int32 size) {
				ENGINE_DEBUG_ASSERT(canWriteMessage(), "Unable to write message");
				auto& blob = writeBlobs.insert(nextBlob);
				blob.type = type;
				blob.data.assign(data, data + size);
				attemptWriteBlob(buff, nextBlob);
				++nextBlob;
			}

			bool recv(const MessageHeader& hdr, BufferReader buff) {
				if (recvData.canInsert(hdr.seq) && !recvData.contains(hdr.seq)) {
					recvData.insert(hdr.seq);

					const byte* dataBegin = buff.peek();

					// TODO: cant just reinterpret, should memcpy, possible alignment issues
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
						// TODO: cant just reinterpret, should memcpy, possible alignment issues
						found->total = *reinterpret_cast<const int32*>(dataBegin);
						dataBegin += sizeof(found->total);
					}

					const auto dataEnd = buff.end();
					// ENGINE_INFO("Recv blob part: ", info.seq(), " ", dataEnd - dataBegin, " ", info.start() & BlobHeader::LEN_MASK);
					found->insert(info.start() & BlobHeader::LEN_MASK, dataBegin, dataEnd);
				}

				return false;
			}

			void attemptWriteBlob(BufferWriter& buff, SeqNum seq) {
				auto* blob = writeBlobs.find(seq);
				if (!blob) { return; }

				while (Base::canWriteMessage()) {
					if (blob->remaining() == 0) { return; }

					// TODO: this should be configurable some where
					// Always leave some room for other messages.
					constexpr static int32 maxPacketUsage = sizeof(Packet::body) - 128;
					const auto space = std::min(maxPacketUsage, std::max(0,
						static_cast<int32>(buff.space())
						- static_cast<int32>(sizeof(MessageHeader))
						- static_cast<int32>(sizeof(BlobHeader))
						- static_cast<int32>(sizeof(int32)) // Optional size field
					));

					if (space < 32) { // Arbitrary minimum data size
						return;
					}

					// We need to use an alias because beginMessage expects an "empty" buffer
					auto alias = buff.alias();
					if (auto msg = Base::beginMessage(*static_cast<Base*>(this), blob->type, alias)) {
						BlobHeader head;
						head.start() = blob->curr | (blob->curr > 0 ? 0 : ~BlobHeader::LEN_MASK);
						head.seq() = seq;
						msg.write(head);

						if (blob->curr == 0) {
							msg.write(static_cast<int32>(blob->data.size()));
						}

						const int32 len = std::min(space, blob->remaining());
						msg.write(blob->data.data() + blob->curr, len);
						// ENGINE_INFO("Write blob part: ", head.seq(), " = ", seq, " ", len);
						blob->curr += len;
						++blob->parts;
						buff.advance(alias.size());
					} else {
						ENGINE_DEBUG_ASSERT(false, "Unable to write to underlying channel. This should not occur.");
					}
				}
			}

			void fill(SeqNum pktSeq, BufferWriter& buff) {
				for (auto seq = writeBlobs.minValid(); Math::Seq::less(seq, writeBlobs.max() + 1); ++seq) {
					attemptWriteBlob(buff, seq);
				}

				Base::fill(pktSeq, buff);
			}

			const MessageHeader* recvNext() {
				for (auto seq = recvBlobs.minValid(); Math::Seq::less(seq, recvBlobs.max() + 1); ++seq) {
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

			void recvPacketAck(SeqNum pktSeq) {
				// TODO: why did we inherit. This is dumb.
				auto* pkt = Base::pktData.find(pktSeq);
				if (!pkt) { return; }

				for (SeqNum s : pkt->messages) {
					if (auto* data = Base::msgData.find(s)) {
						Base::msgData.remove(s);

						// Now remove blobs
						const byte* start = data->data.data();
						start += sizeof(MessageHeader);

						const auto* head = reinterpret_cast<const BlobHeader*>(start);
						const auto seq = head->seq();

						auto* blob = writeBlobs.find(seq);
						if (blob) {
							--blob->parts;
							// ENGINE_INFO("Blob part ack: ", pktSeq, " ", s, " ", seq, " ", blob->parts);
							if (blob->parts == 0 && blob->remaining() == 0) {
								writeBlobs.remove(seq);
								// ENGINE_SUCCESS("Blob complete! ", seq);
							}
						} else {
							// This shouldn't happen?
							ENGINE_ASSERT(false, "Blob not found! ", seq);
							ENGINE_DIE;
						}
					}
				}

				Base::pktData.remove(pktSeq);
			}
	};
}
