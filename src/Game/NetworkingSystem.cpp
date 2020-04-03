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
			const auto size = socket.recv(stream.data(), stream.capacity(), addr);

			if (size != -1) {
				stream.reset(size);
				std::string a;
				std::array<char, 4> b;
				char c[8];
				stream >> a;
				stream >> b;
				stream >> c;
				std::cout << "== recv == "
					<< "\n\tdata: " << stream.data()
					<< "\n\tsize: " << stream.size()
					<< "\n\taddr: " << addr
					<< "\n\t" << a << b.data() << c
					<< stream.read<float32>()
					<< stream.read<char[]>()
					<< stream.read<int32>()
					<< stream.read<char[]>()
					<< "\n";
			}
		} else {
			const Engine::Net::IPv4Address addr = {127,0,0,1, 27015};

			stream.reset(0);
			std::string a = "NetworkingSystem::tick";
			std::array<char, 4> b = {' ', '-', ' ', '\0'};
			char c[8] = "apples ";
			stream
				<< a
				<< b
				<< c
				<< 3.14159001f
				<< " "
				<< 0xFF
				<< " this is a test";

			const auto size = socket.send(addr, reinterpret_cast<const char*>(stream.data()), stream.size());
		}
		Sleep(1000);
	}

	void NetworkingSystem::run(float32 dt) {
	}
}
