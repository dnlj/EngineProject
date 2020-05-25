#pragma once

// Engine
#include <Engine/Net/AckData.hpp>
#include <Engine/Net/Channel.hpp>
#include <Engine/Net/Common.hpp>
#include <Engine/Net/MessageHeader.hpp>

namespace Engine::Net {
	class PacketReader {
		public: // TODO: not public
			char* curr = nullptr;
			char* last = nullptr;

			SequenceNumber nextSeq[static_cast<int32>(Channel::_COUNT)] = {}; // TODO: dont think we need this on reader. Only writer
			AckData channelAckData[2] = {};

		public:
			// TODO: del copy/move constr
			void set(char* curr, char* last);

			// TODO: name
			// TODO: doc
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

			// TODO: doc
			bool next();

			// TODO: move duplicate reader/writer functions into base class?
			int32 size() const;
	};
}

#include <Engine/Net/PacketReader.ipp>
