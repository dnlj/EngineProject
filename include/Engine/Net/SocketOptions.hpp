#pragma once


namespace Engine::Net {
	/**
	 * Options for network sockets.
	 * 
	 * @see https://docs.microsoft.com/en-us/windows/win32/winsock/socket-options
	 * @see https://linux.die.net/man/7/socket
	 */
	enum class SocketOption : int {
		BROADCAST,
		MULTICAST_JOIN,
		MULTICAST_LEAVE,
	};
}
