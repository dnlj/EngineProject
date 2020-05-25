#pragma once

// Engine
#include <Engine/Net/Connection.hpp>
#include <Engine/Utility/Utility.hpp>


namespace Engine::Net {
	Connection::Connection(
		UDPSocket& sock,
		IPv4Address addr,
		Clock::TimePoint lastMessageTime)
		: writer{sock, addr}
		, lastMessageTime{lastMessageTime} {
	}
	
	void Connection::writeRecvAcks(Channel ch) {
		auto& ackData = reader.getAckData(ch);
		writer.write(ch);
		writer.write(ackData.nextAck);
		writer.write(ackData.acks);
	}

	IPv4Address Connection::address() const {
		return writer.addr;
	}
}
