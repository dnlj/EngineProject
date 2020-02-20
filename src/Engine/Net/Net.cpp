// Windows
#if defined(ENGINE_OS_WINDOWS)
	#include <WinSock2.h>
#endif

// Engine
#include <Engine/Net/Net.hpp>


namespace Engine::Net {
	bool startup() {
		WSADATA data;
		auto err = WSAStartup(MAKEWORD(2,2), &data);
		if (err) {
			// TODO: handle
			return false;
		}

		return true;
	}

	// TODO: Doc
	bool shutdown() {
		if (WSACleanup()) {
			// TODO: WSAGetLastError
			return false;
		}

		return true;
	}
}
