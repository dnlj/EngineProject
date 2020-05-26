#pragma once

// Engine
#include <Engine/Net/AckData.hpp>
#include <Engine/Net/Channel.hpp>
#include <Engine/Net/Common.hpp>
#include <Engine/Net/MessageHeader.hpp>

namespace Engine::Net {
	class PacketReader {
		private:
			char* curr = nullptr;
			char* last = nullptr;
			AckData channelAckData[2] = {};
			uint64 bytesRead = 0;

		public:
			PacketReader() = default;
			PacketReader(const PacketReader&) = delete;
			PacketReader(PacketReader&&) = delete;

			uint64 totalBytesRead() const { return bytesRead; }

			void set(char* curr, char* last);

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
			 * Gets the size of the current message.
			 */
			int32 size() const;

			/**
			 * Gets that ack data for the channel @p ch.
			 */
			const AckData& getAckData(Channel ch) const;
	};
}

#include <Engine/Net/PacketReader.ipp>
