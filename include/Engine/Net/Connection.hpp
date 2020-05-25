#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Net/AckData.hpp>
#include <Engine/Net/IPv4Address.hpp>
#include <Engine/Net/MessageHeader.hpp>
#include <Engine/Net/Net.hpp>
#include <Engine/Net/PacketWriter.hpp>
#include <Engine/Clock.hpp>
#include <Engine/StaticVector.hpp>


namespace Engine::Net {
	//////////////////////////////////////////////////////////////////////////////////////////////////
	// TODO: move
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
			int32 size() const { return static_cast<int32>(last - curr); }
	};

	// TODO: make read functions return ptrs? We need better error handling.
	class Connection {
		public:
			PacketReader reader;
			PacketWriter writer;
			Clock::TimePoint lastMessageTime;

		public:
			Connection(UDPSocket& sock, IPv4Address addr = {}, Clock::TimePoint lastMessageTime = {});
			Connection(const Connection&) = delete;
			Connection(const Connection&&) = delete;

			void writeRecvAcks(Channel ch);

			/**
			 * Gets the most recently associated address.
			 * Set from either #reset or #recv.
			 */
			const IPv4Address& address() const; // TODO: is this used anywhere?
	};
}

#include <Engine/Net/Connection.ipp>
