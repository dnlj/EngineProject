// Windows
#if defined(ENGINE_OS_WINDOWS)
	#include <WinSock2.h>
	#include <WS2tcpip.h>
#endif

// STD
#include <regex>

// Engine
#include <Engine/Net/Net.hpp>
#include <Engine/Win32/Win32.hpp>


namespace Engine::Net {
	bool startup() {
		WSADATA data;
		auto err = WSAStartup(MAKEWORD(2,2), &data);
		if (err) { return false; }
		return true;
	}

	bool shutdown() {
		if (WSACleanup()) { return false; }
		return true;
	}
	
	IPv4Address hostToAddress(const std::string& uri) {
		IPv4Address addr = {};
		addrinfo* results = nullptr;
		addrinfo hints = {
			.ai_family = AF_INET, // TODO: support ipv6 - AF_UNSPEC
			.ai_socktype = SOCK_DGRAM,
		};

		static const std::regex regex{R"(^(?:(.*):\/\/)?(.*?)(?::(\d+))?(?:\/.*)?$)"};
		std::smatch matches;
		std::regex_match(uri, matches, regex);
		ENGINE_ASSERT_WARN(matches.size() == 4, "Incorrect number of captures");

		std::string host = matches[2].str();
		std::string serv = matches[3].matched ? matches[3].str() : matches[1].str();

		if (auto err = getaddrinfo(host.data(), serv.data(), &hints, &results); err) {
			ENGINE_WARN("Address error - ", Engine::Win32::getLastErrorMessage());
		} else {
			for (auto ptr = results; ptr; ptr = results->ai_next) {
				if (ptr->ai_family != AF_INET) { continue; }

				addr = *ptr->ai_addr;
				break;
			}
		}

		freeaddrinfo(results);
		return addr;
	}
}
