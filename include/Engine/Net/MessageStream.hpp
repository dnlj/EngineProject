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


namespace Engine::Net {
	class Message {
		public:
			class Header {
				public:
					uint16 protocol;
					uint8 type;
					uint8 flags;
					uint32 sequence;
			};
			static_assert(sizeof(Header) == 8);

			Header header;
			char data[512 - sizeof(Header)];
	};
	static_assert(sizeof(Message) == 512);

	class MessageStream {
		private:
			char* curr;
			char* last;
			Message msg;

		public:
			// TODO: header field operations
			Message::Header& header();
			const Message::Header& header() const;

			int32 recv(const UDPSocket& socket, IPv4Address& addr);
			int32 send(const UDPSocket& socket, const IPv4Address& addr) const;

			void reset(int32 sz);
			void reset();

			int32 size() const;

			int32 capacity() const;

			char* data();

			const char* data() const;

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
