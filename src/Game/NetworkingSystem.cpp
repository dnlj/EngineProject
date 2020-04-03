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
			Engine::Net::IPv4Address addr;
			memset(stream.data(), 0, sizeof(Engine::Net::Message::data)); // TODO: rm - for testing. this shouldnt matter
			const auto size = socket.recv(stream.data(), stream.capacity(), addr);

			if (size != -1) {
				stream.reset(size);
				std::cout << "== recv == "
					<< "\n\tdata: " << stream.data()
					<< "\n\tsize: " << stream.size()
					<< "\n\taddr: " << addr
					<< "\n\t" << stream.read<char[]>()
					<< stream.read<char[]>()
					<< stream.read<float32>()
					<< stream.read<char[]>()
					<< stream.read<int32>()
					<< stream.read<char[]>()
					<< "\n";
			}
		} else {
			const char data[] = "NetworkingSystem::tick";
			const Engine::Net::IPv4Address addr = {127,0,0,1, 27015};

			stream.reset();
			stream << data << " - " << 3.14159001f << " " << 0xFF << " this is a test";

			const auto size = socket.send(addr, reinterpret_cast<const char*>(stream.data()), stream.size());
		}
		Sleep(1000);
	}

	void NetworkingSystem::run(float32 dt) {
	}
}
