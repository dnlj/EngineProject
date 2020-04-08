#pragma once

// STD
#include <array>
#include <type_traits>
#include <cstring>
#include <string>
#include <vector>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Net/UDPSocket.hpp>
#include <Engine/Net/IPv4Address.hpp>

// TODO: split
// TODO: doc
namespace Engine::Net {
	class PacketHeader {
		public:
			uint16 protocol;
	};
	static_assert(sizeof(PacketHeader) == 2);

	class MessageHeader {
		public:
			uint8 type; // TODO: should this be a property of the packet not messages?
			uint8 flags;
			uint16 _filler;
			uint32 sequence;
	};
	static_assert(sizeof(MessageHeader) == 8);

	class Packet {
		public:
			PacketHeader header;
			char data[512 - sizeof(header)];
	};
	static_assert(sizeof(Packet) == 512);

	class MessageStream {
		private:
			UDPSocket* sock;
			IPv4Address* addr;

			char* curr;
			char* last;
			// TODO: add class for this. less error prone.
			Packet packet;

		public:
			static constexpr int32 MAX_MESSAGE_SIZE = 256;

		public:
			MessageStream();
			MessageStream(const MessageStream&) = delete;
			MessageStream(const MessageStream&&) = delete;

			void setSocket(UDPSocket& sock);
			void setAddress(IPv4Address& addr);

			void next();

			// TODO: header field operations
			MessageHeader& header();
			const MessageHeader& header() const;

			char* data();
			const char* data() const;

			int32 recv();
			int32 send() const;

			void reset(int32 sz = 0);

			int32 size() const;

			int32 capacity() const;

			template<class T>
			MessageStream& operator<<(const T& t);

			template<class T>
			MessageStream& operator>>(T& t);

			template<class T>
			void write(const T* t, size_t sz);

			template<class T>
			void write(const T& t);

			template<class T, size_t N>
			void write(const T(&t)[N]);

			template<class T, size_t N>
			void write(const std::array<T, N>& t);

			void write(const std::string& t);

			template<class T> 
			void read(T* t, size_t sz);

			template<class T, size_t N>
			void read(T(&t)[N]);

			template<class T, size_t N>
			void read(std::array<T, N>& t);

			void read(std::string& t);

			template<class T> 
			void read(T& t);

			template<class T> 
			auto read();
	};
}

#include <Engine/Net/MessageStream.ipp>
