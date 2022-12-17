// STD
#include <set>
#include <concepts>
#include <iomanip>
#include <random>
#include <algorithm>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Clock.hpp>
#include <Engine/ECS/Entity.hpp>
#include <Engine/Meta/for.hpp>

// Game
#include <Game/World.hpp>
#include <Game/systems/NetworkingSystem.hpp>
#include <Game/systems/all.hpp> // TODO: any way to get around this? not great for build times
#include <Game/comps/all.hpp>


namespace {
	using PlayerFilter = Engine::ECS::EntityFilterList<
		Game::PlayerFlag,
		Game::ConnectionComponent
	>;

	// TODO: would probably be easier to just have a base class instead of all these type traits
	template<class T, class = void>
	struct GetComponentReplication {
		constexpr static auto value = Engine::Net::Replication::NONE;
	};

	// TODO: figure out a good pattern
	constexpr uint8 MESSAGE_PADDING_DATA[sizeof(Engine::Net::Packet::body) - sizeof(Engine::Net::MessageHeader) - sizeof(uint16)] = {
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33,
	};
}
#if DEBUG
namespace {
	struct HandleMessageDef_DebugBreak_Struct {
		Engine::Net::BufferReader& msg;
		~HandleMessageDef_DebugBreak_Struct() {
			if (msg.remaining() != 0) { __debugbreak(); }
		}
	};
}
#define HandleMessageDef_DebugBreak(MsgType) HandleMessageDef_DebugBreak_Struct _ENGINE_temp_object_for_HandleMessageDef_DebugBreak_Struct{msg}
#else
#define HandleMessageDef_DebugBreak(...)
#endif


#define HandleMessageDef(MsgType) \
	template<> \
	void NetworkingSystem::handleMessageType<MsgType>(EngineInstance& engine, Engine::ECS::Entity ent, Connection& from, const Engine::Net::MessageHeader head, Engine::Net::BufferReader& msg) { \
	if constexpr (!(Engine::Net::MessageTraits<MsgType>::side & ENGINE_SIDE)) { \
		ENGINE_WARN("Message received by wrong side. Aborting."); return; \
	} \
	if (!(Engine::Net::MessageTraits<MsgType>::rstate & from.getState())) { \
		msg.discard(); \
		ENGINE_WARN("Messages received in wrong state. Aborting. ", getMessageName(head.type), "(", (int)head.type, ")"); \
		return; \
	} \
	HandleMessageDef_DebugBreak(#MsgType);


namespace Game {
	HandleMessageDef(MessageType::UNKNOWN)
	}

	HandleMessageDef(MessageType::DISCOVER_SERVER)
		// TODO: rate limit per ip (longer if invalid packet)
		constexpr auto size = sizeof(MESSAGE_PADDING_DATA);
		if (msg.remaining() == size && !memcmp(msg.read(size), MESSAGE_PADDING_DATA, size)) {
			if (auto reply = from.beginMessage<MessageType::SERVER_INFO>()) {
				//constexpr char name[] = "This is the name of the server";
				std::string name = "This is the name of the server ";
				name += std::to_string(Engine::getGlobalConfig().port);
				reply.write<int>(int(std::size(name)));
				reply.write(name.data(), std::size(name));
			}
		} else if constexpr (ENGINE_DEBUG) {
			[[maybe_unused]] const auto rem = msg.remaining();
			[[maybe_unused]] const auto diff = rem - size;
			ENGINE_DEBUG_ASSERT(false);
			msg.discard();
		}
	}
	
	HandleMessageDef(MessageType::SERVER_INFO)
		#if ENGINE_CLIENT
			int len;
			if (!msg.read<int>(&len)) { return; }

			auto& world = engine.getWorld();
			auto& netSys = world.getSystem<NetworkingSystem>();
			const char* name = reinterpret_cast<const char*>(msg.read(len));
			auto& servInfo = netSys.servers[from.address()];
			servInfo.name.assign(name, len);
			servInfo.lastUpdate = world.getTime();
		#endif
	}

	HandleMessageDef(MessageType::CONNECT_REQUEST)
		constexpr auto size = sizeof(MESSAGE_PADDING_DATA);
		if (msg.remaining() != size + sizeof(uint16) || memcmp(msg.read(size), MESSAGE_PADDING_DATA, size)) {
			// TODO: rate limit connections with invalid messages
			ENGINE_WARN("Got invalid connection request from ", from.address());
			return;
		}

		uint16 remote = {};
		msg.read<uint16>(&remote);

		if (from.getKeyRemote() && from.getKeyRemote() != remote) {
			ENGINE_WARN("Got connection request with invalid key from ", from.address(), " (", remote, " != ", from.getKeyRemote(), ")");
			return msg.discard();
		} else {
			from.setKeyRemote(remote);
		}

		if (from.getState() == ConnectionState::Disconnected) {
			ENGINE_DEBUG_ASSERT(from.getKeyLocal() == 0, "It should not be possible to have a local key in an unconnected state. This is a bug.");
			from.setState(ConnectionState::Connecting);

			auto& world = engine.getWorld();
			auto& netSys = world.getSystem<NetworkingSystem>(); // TODO: make a genKeys that doesnt use networking system
			from.setKeyLocal(netSys.genKey());
		} else {
			ENGINE_DEBUG_ASSERT(from.getKeyLocal() != 0, "It should not be possible to be missing a local key in any non-unconnected state. This is a bug.");
		}

		if (auto reply = from.beginMessage<MessageType::CONNECT_CHALLENGE>()) {
			reply.write(from.getKeyLocal());
			ENGINE_LOG("CONNECT_REQUEST from ", from.address(), " lkey: ", from.getKeyLocal(), " rkey: ", from.getKeyRemote());
		}
	}
	
	HandleMessageDef(MessageType::CONNECT_CHALLENGE)
		uint16 remote = {};
		msg.read<uint16>(&remote);

		if (from.getKeyRemote()) {
			ENGINE_WARN("Extraneous CONNECT_CHALLENGE received. Ignoring.");
			ENGINE_DEBUG_ASSERT(from.getState() == ConnectionState::Connected);
			return msg.discard();
		}

		if (auto reply = from.beginMessage<MessageType::CONNECT_CONFIRM>()) {
			reply.write(remote);
			from.setKeyRemote(remote);
			from.setState(ConnectionState::Connected);
			ENGINE_LOG("CONNECT_CHALLENGE from ", from.address(), " lkey: ", from.getKeyLocal(), " rkey: ", from.getKeyRemote());
		} else [[unlikely]] {
			ENGINE_WARN("Unable to send CONNECT_CONFIRM");
		}
	}

	HandleMessageDef(MessageType::CONNECT_CONFIRM)
		uint16 key = {};
		msg.read<uint16>(&key);

		if (key != from.getKeyLocal()) {
			ENGINE_WARN("CONNECT_CONFIRM: Invalid recv key ", key, " != ", from.getKeyLocal());
			return msg.discard();
		}
		
		auto& world = engine.getWorld();
		auto& netSys = world.getSystem<NetworkingSystem>();

		ENGINE_LOG("CONNECT_CONFIRM from ", from.address(), " lkey: ", from.getKeyLocal(), " rkey: ", from.getKeyRemote(), " tick: ", world.getTick());
		from.setState(ConnectionState::Connected);

		netSys.addPlayer(ent);

		// TODO: change message type of this (for client). This isnt a confirmation this is initial sync or similar.
		if (auto reply = from.beginMessage<MessageType::ECS_INIT>()) {
			reply.write(ent);
			reply.write(world.getTick());
		}

		if (auto reply = from.beginMessage<MessageType::CONFIG_NETWORK>()) {
			reply.write(from.getPacketRecvRate());
		}
	}

	HandleMessageDef(MessageType::DISCONNECT)
		if (from.getState() != ConnectionState::Connected) { return; }
	
		auto& world = engine.getWorld();
		auto& netSys = world.getSystem<NetworkingSystem>();
		auto& connComp = world.getComponent<ConnectionComponent>(ent);

		// TODO: restructure so we dont need conncomp
		if (connComp.disconnectAt != Engine::Clock::TimePoint{}) { return; }
		ENGINE_LOG("MessageType::DISCONNECT ", from.address(), " ", ent);
		netSys.disconnect(ent, connComp);
	}

	HandleMessageDef(MessageType::PING)
		static uint8 last = 0;

		uint8 data;
		msg.read(&data);

		const bool pong = data & 0x80;
		const int32 val = data & 0x7F;
		last = val;

		if (pong) {
			ENGINE_LOG("recv pong @ ",
				std::fixed, std::setprecision(2),
				Engine::Clock::now().time_since_epoch().count() / 1E9,
				" from ", from.address(),
				" ", val
			);
		} else {
			ENGINE_LOG("recv ping @ ",
				std::fixed, std::setprecision(2),
				Engine::Clock::now().time_since_epoch().count() / 1E9,
				" from ", from.address(),
				" ", val
			);
			if (auto reply = from.beginMessage<MessageType::PING>()) {
				reply.write(static_cast<uint8>(val | 0x80));
			}
		}
	}

	HandleMessageDef(MessageType::CONFIG_NETWORK)
		float32 rate;
		if (!msg.read(&rate)) { return; }

		if constexpr (ENGINE_CLIENT) {
			if (auto reply = from.beginMessage<MessageType::CONFIG_NETWORK>()) {
				reply.write(from.getPacketRecvRate());
			}
		}

		// TODO: these values should be configured by convar/config
		constexpr float32 maxSendRate = 256;
		constexpr float32 minSendRate = 8;
		float32 r2 = rate;

		// We need this check because MSVC does not handle comparisons correctly for non-finite values even when is_iec559 is true.
		if (!std::isfinite(r2)) {
			r2 = minSendRate;
		}

		ENGINE_LOG("Network send rate updated: ", r2);
		from.setPacketSendRate(std::max(minSendRate, std::min(r2, maxSendRate)));
	}
}
#undef HandleMessageDef

namespace Game {
	namespace Net = Engine::Net;
	NetworkingSystem::NetworkingSystem(SystemArg arg)
		: System{arg}
		, group{Engine::getGlobalConfig().group}
		, socket{ENGINE_SERVER ? Engine::getGlobalConfig().port : 0, Engine::Net::SocketFlag::NonBlocking}
		#if ENGINE_SERVER
		, discoverServerSocket{Net::UDPSocket::doNotInitialize}
		#endif
		, rng{pcg_extras::seed_seq_from<std::random_device>{}} {

		ENGINE_LOG("Listening on port ", socket.getAddress().port);

		setMessageHandler(MessageType::UNKNOWN, handleMessageType<MessageType::UNKNOWN>);
		setMessageHandler(MessageType::DISCOVER_SERVER, handleMessageType<MessageType::DISCOVER_SERVER>);
		setMessageHandler(MessageType::SERVER_INFO, handleMessageType<MessageType::SERVER_INFO>);
		setMessageHandler(MessageType::CONNECT_REQUEST, handleMessageType<MessageType::CONNECT_REQUEST>);
		setMessageHandler(MessageType::CONNECT_CHALLENGE, handleMessageType<MessageType::CONNECT_CHALLENGE>);
		setMessageHandler(MessageType::DISCONNECT, handleMessageType<MessageType::DISCONNECT>);
		setMessageHandler(MessageType::CONNECT_CONFIRM, handleMessageType<MessageType::CONNECT_CONFIRM>);
		setMessageHandler(MessageType::PING, handleMessageType<MessageType::PING>);
		setMessageHandler(MessageType::CONFIG_NETWORK, handleMessageType<MessageType::CONFIG_NETWORK>);

		#if ENGINE_SERVER
		if (group.port) {
			if (group.port == socket.getAddress().port) {
				ENGINE_WARN("The server port and multicast port should not be the same value(", group.port, "). May lead to instability.");
			} else {
				discoverServerSocket = Net::UDPSocket{group.port, Net::SocketFlag::NonBlocking | Net::SocketFlag::ReuseAddress};
				if (discoverServerSocket.setOption<Net::SocketOption::MulticastJoin>(group)) {
					ENGINE_LOG("LAN server discovery is available. Joining multicast group ", group);
				} else {
					ENGINE_WARN("LAN server discovery is unavailable; Unable to join multicast group ", group);
				}
			}
		} else {
			ENGINE_LOG("LAN server discovery is disabled");
		}
		#endif
	}

	#if ENGINE_CLIENT
	void NetworkingSystem::broadcastDiscover() {
		const auto now = world.getTime();
		for (auto it = servers.begin(); it != servers.end(); ++it) {
			if (it->second.lastUpdate + std::chrono::seconds{5} < now) {
				servers.erase(it);
			}
		}

		const auto ent = getOrCreateEntity(group);
		const auto& conn = world.getComponent<ConnectionComponent>(ent).conn;
		if (auto msg = conn->beginMessage<MessageType::DISCOVER_SERVER>()) {
			msg.write(MESSAGE_PADDING_DATA);
		}
	}
	#endif

	void NetworkingSystem::recvAndDispatchMessages(Engine::Net::UDPSocket& sock) {
		int32 sz;
		while ((sz = sock.recv(&packet, sizeof(packet), address)) > -1) {
			const auto ent = getOrCreateEntity(address);
			auto& connComp = world.getComponent<ConnectionComponent>(ent);
			const auto& conn = connComp.conn;
			// TODO: move back to connection
			if (packet.getProtocol() != Engine::Net::protocol) {
				ENGINE_WARN("Invalid protocol"); // TODO: rm - could be used for lag/dos?
				continue;
			}

			if (conn->getKeyLocal() != packet.getKey()) {
				if (conn->getState() == ConnectionState::Connected) {
					ENGINE_WARN("Invalid key for ", conn->address(), " ", packet.getKey(), " != ", conn->getKeyLocal());
					continue;
				}
			}

			// ENGINE_LOG("****** ", conn->getKeySend(), " ", conn->getKeyRecv(), " ", packet.getKey(), " ", packet.getSeqNum());
			
			if (!conn->recv(packet, sz, now)) { continue; }

			while (true) {
				auto [hdr, msg] = conn->recvNext();
				if (hdr.type == 0) { break; }
				dispatchMessage(ent, connComp, hdr, msg);
			}
		}
	}

	void NetworkingSystem::update(float32 dt) {
		now = world.getTime();

		// Recv messages
		#if ENGINE_SERVER
			recvAndDispatchMessages(discoverServerSocket);
		#endif
		recvAndDispatchMessages(socket);

		// TODO: instead of sending all connections on every X. Send a smaller number every frame to distribute load.

		// Send messages
		// TODO: rate should be configurable somewhere
		const bool shouldUpdate = now - lastUpdate >= std::chrono::milliseconds{1000 / 20};

		if (shouldUpdate) {
			lastUpdate = now;
			if constexpr (ENGINE_CLIENT) { updateClient(); }

			for (const auto ent : world.getFilter<ConnectionComponent>()) {
				auto& connComp = world.getComponent<ConnectionComponent>(ent);
				const auto diff = now - connComp.conn->recvTime();
				if (diff > timeout) {
					ENGINE_LOG("Connection for ", ent ," (", connComp.conn->address(), ") timed out.");
					connComp.disconnectAt = now;
				}
			}
		}

		// Send Ack messages & unacked
		for (const auto ent : world.getFilterAll<true, ConnectionComponent>()) {
			auto& connComp = world.getComponent<ConnectionComponent>(ent);
			auto& conn = *connComp.conn;

			if (connComp.disconnectAt != Engine::Clock::TimePoint{}) {
				if (conn.getState() == ConnectionState::Disconnecting) {
					ENGINE_LOG("Send DISCONNECT to ", connComp.conn->address());
					if (auto msg = conn.beginMessage<MessageType::DISCONNECT>()) {
					}
				}

				if (connComp.disconnectAt <= now) {
					conn.send(socket);
					conn.setKeyLocal(0);
					conn.setKeyRemote(0);
					conn.setState(ConnectionState::Disconnected);

					ENGINE_LOG("Disconnected ", ent, " ", connComp.conn->address());
					addressToEntity.erase(addressToEntity.find(connComp.conn->address()));
					world.getComponent<ConnectionComponent>(ent).conn.reset(); // Make sure connection is closed now and not later after defered destroy
					world.deferedDestroyEntity(ent);

					#if ENGINE_CLIENT
						// TODO (uAiwkWDY): really would like a better way to handle this kind of stuff. event/signal system maybe.
						world.getSystem<EntityNetworkingSystem>().getRemoteToLocalEntityMapping().clear();
					#endif
					continue;
				}
			}

			conn.send(socket);
		}

		#ifdef ENGINE_UDP_NETWORK_SIM
			socket.realSimSend();
		#endif
	}

	void NetworkingSystem::updateClient() {
		for (const auto ent : world.getFilter<ConnectionComponent>()) {
			const auto& conn = *world.getComponent<ConnectionComponent>(ent).conn;
			if (conn.getState() == ConnectionState::Connecting) {
				connectTo(conn.address());
				break;
			} else if (conn.getState() == ConnectionState::Connected) {
				static uint8 ping = 0;
				static auto next = now;
				if (next > now) { return; }
				next = now + std::chrono::milliseconds{5000};
		
				for (auto& ply : world.getFilter<PlayerFilter>()) {
					auto& conn2 = *world.getComponent<ConnectionComponent>(ply).conn;
					if (auto msg = conn2.beginMessage<MessageType::PING>()) {
						msg.write(static_cast<uint8>(++ping & 0x7F));
					}
				}
			}
		}
	}

	int32 NetworkingSystem::connectionsCount() const {
		return static_cast<int32>(world.getFilter<ConnectionComponent>().size());
	}

	int32 NetworkingSystem::playerCount() const {
		return static_cast<int32>(world.getFilter<PlayerFilter>().size());
	}

	void NetworkingSystem::disconnect(Engine::ECS::Entity ent, ConnectionComponent& connComp) {
		connComp.conn->setState(ConnectionState::Disconnected);
		connComp.disconnectAt = world.getTime() + disconnectTime;
		world.setEnabled(ent, false);
	}
	
	void NetworkingSystem::requestDisconnect(const Engine::Net::IPv4Address& addr) {
		const auto ent = getEntity(addr);
		if (ent == Engine::ECS::INVALID_ENTITY) { return; }

		auto& connComp = world.getComponent<ConnectionComponent>(ent);
		disconnect(ent, connComp);
		connComp.conn->setState(ConnectionState::Disconnecting);
		ENGINE_INFO("Request disconnect ", addr, " ", ent);
	}

	void NetworkingSystem::connectTo(const Engine::Net::IPv4Address& addr) {
		// TODO: the client should only ever connect to one server
		const auto ent = getOrCreateEntity(addr);
		auto& conn = world.getComponent<ConnectionComponent>(ent).conn;

		if (conn->getState() != ConnectionState::Disconnected) {
			ENGINE_WARN("Attempting to connect to same server while already connected. Aborting.");
			return;
		}

		conn->setState(ConnectionState::Connecting);

		if (!conn->getKeyLocal()) {
			conn->setKeyLocal(genKey());
		}
		ENGINE_LOG("TRY CONNECT TO: ", addr, " lkey: ", conn->getKeyLocal(), " rkey: ",  conn->getKeyRemote(), " Tick: ", world.getTick());

		if (auto msg = conn->beginMessage<MessageType::CONNECT_REQUEST>()) {
			msg.write(MESSAGE_PADDING_DATA);
			msg.write(conn->getKeyLocal());
		}
	}

	void NetworkingSystem::addPlayer(const Engine::ECS::Entity ent) {
		// TODO: i feel like this should be handled elsewhere. Where?

		ENGINE_INFO("Add player: ", ent, " ", world.hasComponent<PlayerFlag>(ent), " Tick: ", world.getTick());
		auto& physSys = world.getSystem<PhysicsSystem>();

		if constexpr (ENGINE_SERVER) {
			world.addComponent<NetworkedFlag>(ent);
			world.addComponent<ECSNetworkingComponent>(ent);
			world.addComponent<MapAreaComponent>(ent);
		} else {
			world.addComponent<CameraTargetFlag>(ent);
		}
		// TODO: client only
		world.addComponent<PhysicsInterpComponent>(ent);

		world.addComponent<PlayerFlag>(ent);
		auto& spriteComp = world.addComponent<SpriteComponent>(ent);
		spriteComp.path = "assets/player.png";
		spriteComp.texture = engine.getTextureLoader().get2D(spriteComp.path);

		{
			// TODO: query map system and find good spawn location
			const b2Vec2 pos = {0, 2};
			auto& physComp = world.addComponent<PhysicsBodyComponent>(ent);
			physComp.setBody(physSys.createPhysicsCircle(ent, pos, -+PhysicsType::Player));
			physComp.type = PhysicsType::Player;
		}

		world.addComponent<ActionComponent>(ent, world.getTick());
		world.addComponent<MapEditComponent>(ent);
	}

	Engine::ECS::Entity NetworkingSystem::addConnection(const Engine::Net::IPv4Address& addr) {
		auto ent = world.createEntity();
		ENGINE_INFO("Add connection: ", ent, " ", addr, " ", world.hasComponent<PlayerFlag>(ent), " ");
		auto [it, suc] = addressToEntity.emplace(addr, ent);
		auto& connComp = world.addComponent<ConnectionComponent>(ent);
		connComp.conn = std::make_unique<Connection>(addr, now);
		connComp.conn->setState(ConnectionState::Disconnected);
		return ent;
	}
	
	Engine::ECS::Entity NetworkingSystem::getEntity(const Engine::Net::IPv4Address& addr) {
		const auto found = addressToEntity.find(addr);
		if (found == addressToEntity.end()) {
			return Engine::ECS::INVALID_ENTITY;
		}
		return found->second;
	}

	Engine::ECS::Entity NetworkingSystem::getOrCreateEntity(const Engine::Net::IPv4Address& addr) {
		const auto found = addressToEntity.find(addr);
		if (found == addressToEntity.end()) {
			return addConnection(addr);
		} else {
			return found->second;
		}
	}

	void NetworkingSystem::dispatchMessage(Engine::ECS::Entity ent, ConnectionComponent& connComp, const Engine::Net::MessageHeader hdr, Engine::Net::BufferReader& msg) {
		auto& from = *connComp.conn;

		if (hdr.type <= 0 || hdr.type >= MessageType::_count) {
			ENGINE_WARN("Attempting to dispatch invalid message type ", getMessageName(hdr.type), " (", +hdr.type, ")");
		} else {
			auto func = msgHandlers[hdr.type];
			ENGINE_DEBUG_ASSERT(func, "No message handler set for type ", getMessageName(hdr.type), " (", +hdr.type, ")");
			if (func) { func(engine, ent, from, hdr, msg); }
		}

		if (auto rem = msg.remaining(); rem > 0) {
			ENGINE_WARN("Incomplete read of network message ", getMessageName(hdr.type), " (", rem, " bytes remaining). Ignoring.");
		} else if (rem < 0) {
			ENGINE_WARN("Read past end of network messge type ", getMessageName(hdr.type)," (", rem, " bytes remaining).");
		}
	}
}
