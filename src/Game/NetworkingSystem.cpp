// Engine
#include <Engine/Clock.hpp>

// Game
#include <Game/NetworkingSystem.hpp>


namespace Game {
	NetworkingSystem::NetworkingSystem(SystemArg arg)
		: System{arg}
		, socket{ENGINE_SERVER ? 27015 : 0}{
	}

	void NetworkingSystem::setup() {
	}

	void NetworkingSystem::tick(float32 dt) {
		if constexpr (ENGINE_SERVER) {
			char data[512] = {};
			Engine::Net::IPv4Address addr;
			const auto size = socket.recv(data, sizeof(data), addr);
			std::cout << "== recv == "
				<< "\n\tdata: " << data
				<< "\n\tsize: " << size
				<< "\n\taddr: " << addr
				<< "\n";
		} else {
			const char data[] = "NetworkingSystem::tick - ";
			const Engine::Net::IPv4Address addr = {127,0,0,1, 27015};
			const auto size = socket.send(addr, data, sizeof(data));
			std::cout << "== send == "
				<< "\n\tdata: " << data
				<< "\n\tsize: " << size
				<< "\n\taddr: " << addr
				<< "\n";
		}
		Sleep(1000);
	}

	void NetworkingSystem::run(float32 dt) {
	}
}
