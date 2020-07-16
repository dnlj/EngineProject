#pragma once

// Engine
#include <Engine/Net/AckData.hpp>
#include <Engine/Net/Channel.hpp>
#include <Engine/Net/Common.hpp>
#include <Engine/Net/MessageHeader.hpp>

namespace Engine::Net {
	class PacketReader {
		private:
			/** The current position in the packet. */
			char* curr = nullptr;

			/** End of message. Set by user. */
			char* stop = nullptr;

			/** End of packet */
			char* last = nullptr;

			AckData channelAckData[2] = {};
			uint64 bytesRead = 0;

		public:
			PacketReader() = default;
			PacketReader(const PacketReader&) = delete;
			PacketReader(PacketReader&&) = delete;

			uint64 totalBytesRead() const { return bytesRead; }

			void set(char* curr, char* last);
			void setMessageSize(int32 sz) { stop = curr + sz; }

			/**
			 * Updates message acknowledgements.
			 * @return True if the message should be processes; otherwise returns false.
			 */
			bool updateRecvAcks(const MessageHeader& hdr);

			/**
			 * Reads a specific number of bytes from the current message.
			 */
			const void* read(size_t sz);
			
			/**
			 * Reads an object from the current message.
			 */
			template<class T>
			decltype(auto) read();

			/**
			 * Checks if there are any messages to read.
			 */
			bool next();
			
			/**
			 * The number of bytes remaining in the current message.
			 */
			int32 messageSize() const { return static_cast<int32>(stop - curr); };

			/**
			 * The number of bytes remaining in the current packet.
			 */
			int32 size() const;

			/**
			 * Gets that ack data for the channel @p ch.
			 */
			const AckData& getAckData(Channel ch) const;
	};
}

#include <Engine/Net/PacketReader.ipp>
