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
#include <Engine/Meta/ForEach.hpp>

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
		const Engine::Net::MessageHeader* hdr;
		Game::Connection& from;
		const char* msgType = nullptr;

		~HandleMessageDef_DebugBreak_Struct() {
			using byte = Engine::byte;
			const byte* stop = reinterpret_cast<const byte*>(hdr) + sizeof(*hdr) + hdr->size;
			const byte* curr = static_cast<const byte*>(from.read(0));
			const auto rem = stop - curr;
			//if (rem != 0) { __debugbreak(); }
		}
	};
}
#define HandleMessageDef_DebugBreak(MsgType) HandleMessageDef_DebugBreak_Struct _ENGINE_temp_object_for_HandleMessageDef_DebugBreak_Struct{&head, from, MsgType}
#else
#define HandleMessageDef_DebugBreak(...)
#endif

#define HandleMessageDef(MsgType)\
	template<> void NetworkingSystem::handleMessageType<MsgType>(Engine::ECS::Entity ent, ConnectionComponent& connComp, Connection& from, const Engine::Net::MessageHeader& head) {\
	if constexpr (!(Engine::Net::MessageTraits<MsgType>::side & ENGINE_SIDE)) { ENGINE_WARN("Message received by wrong side. Aborting."); return; }\
	if (!(Engine::Net::MessageTraits<MsgType>::state & from.getState())) { from.read(from.recvMsgSize()); ENGINE_WARN("Messages received in wrong state. Aborting. ", getMessageName(head.type), "(", (int)head.type, ")"); return; }\
	HandleMessageDef_DebugBreak(#MsgType);

namespace Game {
	HandleMessageDef(MessageType::UNKNOWN)
	}

	HandleMessageDef(MessageType::DISCOVER_SERVER)
		// TODO: rate limit per ip (longer if invalid packet)
		constexpr auto size = sizeof(MESSAGE_PADDING_DATA);
		if (from.recvMsgSize() == size && !memcmp(from.read(size), MESSAGE_PADDING_DATA, size)) {
			if (auto msg = from.beginMessage<MessageType::SERVER_INFO>()) {
				//constexpr char name[] = "This is the name of the server";
				std::string name = "This is the name of the server ";
				name += std::to_string(Engine::getGlobalConfig().port);
				msg.write<int>(int(std::size(name)));
				msg.write(name.data(), std::size(name));
			}
		}
	}
	
	HandleMessageDef(MessageType::SERVER_INFO)
		#if ENGINE_CLIENT
			auto& servInfo = servers[from.address()];
			const auto* len = from.read<int>();
			const char* name = static_cast<const char*>(from.read(*len));
			servInfo.name.assign(name, *len);
			servInfo.lastUpdate = Engine::Clock::now();
		#endif
	}

	HandleMessageDef(MessageType::CONNECT_REQUEST)
		constexpr auto size = sizeof(MESSAGE_PADDING_DATA);
		if (from.recvMsgSize() != size + sizeof(uint16) || memcmp(from.read(size), MESSAGE_PADDING_DATA, size)) {
			// TODO: rate limit connections with invalid messages
			ENGINE_WARN("Got invalid connection request from ", from.address());
			return;
		}

		from.setState(std::max(from.getState(), ConnectionState::Connecting));
		const auto keySend = *from.read<uint16>();
		if (!from.getKeySend()) {
			from.setKeySend(keySend);
		}
		if (!from.getKeyRecv()) {
			from.setKeyRecv(genKey());
		}
		if (auto msg = from.beginMessage<MessageType::CONNECT_CHALLENGE>()) {
			msg.write(from.getKeyRecv());
			ENGINE_LOG("MessageType::CONNECT_REQUEST ", from.getKeyRecv(), " ", (int)from.getState());
		}
	}
	
	HandleMessageDef(MessageType::CONNECT_CHALLENGE)
		const auto& keySend = *from.read<uint16>();
		if (!from.getKeySend()) {
			from.setKeySend(keySend);
		}
		if (auto msg = from.beginMessage<MessageType::CONNECT_CONFIRM>()) {
			msg.write(from.getKeySend());
			from.setState(ConnectionState::Connected);
		}
		ENGINE_LOG("MessageType::CONNECT_CHALLENGE from ", from.address(), " ", keySend);
	}

	HandleMessageDef(MessageType::CONNECT_CONFIRM)
		const auto key = *from.read<uint16>();
		if (key != from.getKeyRecv()) {
			ENGINE_WARN("CONNECT_CONFIRM: Invalid recv key ", key, " != ", from.getKeyRecv());
			return;
		}

		from.setState(ConnectionState::Connected);
		addPlayer(ent);

		ENGINE_LOG("SERVER MessageType::CONNECT_CONFIRM", " Tick: ", world.getTick());
		// TODO: change message type of this (for client). This isnt a confirmation this is initial sync or similar.
		if (auto msg = from.beginMessage<MessageType::ECS_INIT>()) {
			msg.write(ent);
			msg.write(world.getTick());
		}

		if (auto msg = from.beginMessage<MessageType::CONFIG_NETWORK>()) {
			msg.write(from.getPacketRecvRate());
		}
	}

	HandleMessageDef(MessageType::ECS_INIT)
		auto* remote = from.read<Engine::ECS::Entity>();
		auto* tick = from.read<Engine::ECS::Tick>();

		if (!remote) {
			ENGINE_WARN("Server didn't send remote entity. Unable to sync.");
			return;
		}

		if (!tick) {
			ENGINE_WARN("Unable to sync ticks.");
			return;
		}

		addPlayer(ent);
		entToLocal[*remote] = ent;
		ENGINE_LOG("ECS_INIT - Remote: ", *remote, " Local: ", ent, " Tick: ", world.getTick(), " - ", *tick);

		// TODO: use ping, loss, etc to pick good offset value. We dont actually have good quality values for those stats yet at this point.
		world.setNextTick(*tick + 16);
	}

	HandleMessageDef(MessageType::DISCONNECT)
		if (from.getState() != ConnectionState::Connected) { return; }
		if (connComp.disconnectAt != Engine::Clock::TimePoint{}) { return; }
		ENGINE_LOG("MessageType::DISCONNECT ", from.address(), " ", ent);
		connComp.disconnectAt = Engine::Clock::now() + disconnectTime;
		from.setState(ConnectionState::Disconnected);
	}

	HandleMessageDef(MessageType::PING)
		static uint8 last = 0;
		// TODO: nullptr check
		const auto data = *from.read<uint8>();
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
			if (auto msg = from.beginMessage<MessageType::PING>()) {
				msg.write(static_cast<uint8>(val | 0x80));
			}
		}
	}
	
	HandleMessageDef(MessageType::ECS_ENT_CREATE)
		auto* remote = from.read<Engine::ECS::Entity>();
		if (!remote) { return; }

		auto& local = entToLocal[*remote];
		if (local == Engine::ECS::INVALID_ENTITY) {
			local = world.createEntity();
		}

		world.addComponent<NetworkedFlag>(local);
		ENGINE_LOG("Networked: ", local, world.hasComponent<NetworkedFlag>(local));

		ENGINE_LOG("ECS_ENT_CREATE - Remote: ", *remote, " Local: ", local, " Tick: ", world.getTick());

		// TODO: components init
	}

	HandleMessageDef(MessageType::CONFIG_NETWORK)
		const auto* rate = from.read<float32>();
		if (!rate) { return; }

		if constexpr (ENGINE_CLIENT) {
			if (auto msg = from.beginMessage<MessageType::CONFIG_NETWORK>()) {
				msg.write(from.getPacketRecvRate());
			}
		}

		// TODO: these values should be configured by convar/config
		constexpr float32 maxSendRate = 256;
		constexpr float32 minSendRate = 8;
		float32 r2 = *rate;

		// We need this check because MSVC does not handle comparisons correctly for non-finite values even when is_iec559 is true.
		if (!std::isfinite(r2)) {
			r2 = minSendRate;
		}

		ENGINE_LOG("Network send rate updated: ", r2);
		from.setPacketSendRate(std::max(minSendRate, std::min(r2, maxSendRate)));
	}

	HandleMessageDef(MessageType::ECS_ENT_DESTROY) 
		auto* remote = from.read<Engine::ECS::Entity>();
		if (!remote) { return; }
		auto found = entToLocal.find(*remote);
		if (found != entToLocal.end()) {
			world.deferedDestroyEntity(found->second);
			entToLocal.erase(found);
		}
	}

	HandleMessageDef(MessageType::ECS_COMP_ADD)
		const auto* remote = from.read<Engine::ECS::Entity>();
		const auto* cid = from.read<Engine::ECS::ComponentId>();
		if (!remote || !cid) { return; }

		auto found = entToLocal.find(*remote);
		if (found == entToLocal.end()) { return; }
		auto local = found->second;

		world.callWithComponent(*cid, [&]<class C>(){
			if constexpr (IsNetworkedComponent<C>) {
				if (!world.hasComponent<C>(local)) {
					auto& comp = world.addComponent<C>(local);
					comp.netFromInit(engine, world, local, from);
				}
			} else {
				ENGINE_WARN("Attemping to network non-network component");
			}
		});
	}

	HandleMessageDef(MessageType::ECS_COMP_ALWAYS)
		const auto* remote = from.read<Engine::ECS::Entity>();
		const auto* cid = from.read<Engine::ECS::ComponentId>();
		if (!remote || !cid) { return; }
		
		auto found = entToLocal.find(*remote);
		if (found == entToLocal.end()) { return; }
		auto local = found->second;

		if (!world.isAlive(local)) {
			ENGINE_WARN("Attempting to update dead entitiy ", local);
			return;
		}

		if (!world.hasComponent(local, *cid)) {
			ENGINE_WARN(local, " does not have component ", *cid);
			return;
		}

		world.callWithComponent(*cid, [&]<class C>(){
			if constexpr (IsNetworkedComponent<C>) {
				// TODO: this is a somewhat strange way to handle this
				if constexpr (Engine::ECS::IsSnapshotRelevant<C>::value) {
					const auto* tick = from.read<Engine::ECS::Tick>();
					if (!tick) {
						ENGINE_WARN("No tick specified for snapshot component in ECS_COMP_ALWAYS");
						return;
					}

					auto& state = world.getComponentState<C>(local, *tick);
					state.netFrom(from);
				} else {
					world.getComponent<C>(local).netFrom(from);
				}
			} else {
				ENGINE_WARN("Attemping to network non-network component");
			}
		});
	}

	HandleMessageDef(MessageType::ECS_FLAG)
		const auto remote = from.read<Engine::ECS::Entity>();
		const auto flags = from.read<Engine::ECS::ComponentBitset>();
		if (!remote || !flags) { return; }

		auto found = entToLocal.find(*remote);
		if (found == entToLocal.end()) { return; }
		auto local = found->second;

		Engine::Meta::ForEachIn<ComponentsSet>::call([&]<class C>() {
			constexpr auto cid = world.getComponentId<C>();
			if constexpr (!World::IsFlagComponent<C>::value) { return; }
			if (!flags->test(cid)) { return; }

			if (world.hasComponent<C>(local)) {
				world.removeComponent<C>(local);
			} else {
				world.addComponent<C>(local);
			}
		});
	}
	
	HandleMessageDef(MessageType::PLAYER_DATA)
		const auto* tick = from.read<Engine::ECS::Tick>();
		const auto* trans = from.read<b2Transform>();
		const auto* vel = from.read<b2Vec2>();
		// TODO: angVel

		if (!tick || !trans || !vel) {
			ENGINE_WARN("Invalid PLAYER_DATA network message");
			return;
		}

		if (!world.hasComponent<PhysicsBodyComponent>(ent)) {
			ENGINE_WARN("PLAYER_DATA message received for entity that has no PhysicsBodyComponent");
			return;
		}

		auto& physCompState = world.getComponentState<PhysicsBodyComponent>(ent, *tick);
		const auto diff = physCompState.trans.p - trans->p;
		const float32 eps = 0.0001f; // TODO: figure out good eps value. Probably half the size of a pixel or similar.
		//if (diff.LengthSquared() > 0.0001f) { // TODO: also check q
		// TODO: why does this ever happen with only one player connected?
		if (diff.LengthSquared() >  eps * eps) { // TODO: also check q
			ENGINE_INFO(std::setprecision(std::numeric_limits<decltype(physCompState.trans.p.x)>::max_digits10),
				"Oh boy a mishap has occured on tick ", *tick,
				" (<", physCompState.trans.p.x, ", ", physCompState.trans.p.y, "> - <",
				trans->p.x, ", ", trans->p.y, "> = <",
				diff.x, ", ", diff.y,
				">)"
			);

			physCompState.trans = *trans;
			physCompState.vel = *vel;
			physCompState.rollbackOverride = true;

			world.scheduleRollback(*tick);
		}

		// TODO: vel
	}

	HandleMessageDef(MessageType::ACTION)
		world.getSystem<ActionSystem>().recvActions(from, head, ent);
	}

	HandleMessageDef(MessageType::TEST)
	}

	HandleMessageDef(MessageType::MAP_CHUNK)
		world.getSystem<MapSystem>().chunkFromNet(from, head);
	}

	// TODO: unsued?
	HandleMessageDef(MessageType::SPELL)
		auto& spellSys = world.getSystem<CharacterSpellSystem>();
		const auto* pos = from.read<b2Vec2>();
		const auto* dir = from.read<b2Vec2>();
		if (!pos || !dir) { return; }
		spellSys.queueMissile(*pos, *dir);
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
		const auto now = Engine::Clock::now();
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

			if (conn->getKeyRecv() != packet.getKey()) {
				if (conn->getState() == ConnectionState::Connected) {
					ENGINE_WARN("Invalid key for ", conn->address(), " ", packet.getKey(), " != ", conn->getKeyRecv());
					continue;
				}
			}

			// ENGINE_LOG("****** ", conn->getKeySend(), " ", conn->getKeyRecv(), " ", packet.getKey(), " ", packet.getSeqNum());
			
			if (!conn->recv(packet, sz, now)) { continue; }

			const Engine::Net::MessageHeader* hdr; 
			while (hdr = conn->recvNext()) {
				dispatchMessage(ent, connComp, hdr);
			}
		}
	}

	void NetworkingSystem::update(float32 dt) {
		now = Engine::Clock::now();

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

			//if (info.state == ConnectionState::Connected) {
			//	conn.msgBegin<MessageType::TEST>();
			//	conn.msgEnd<MessageType::TEST>();
			//}

			if (connComp.disconnectAt != Engine::Clock::TimePoint{}) {
				world.setEnabled(ent, false);

				ENGINE_DEBUG_ASSERT(conn.getState() != ConnectionState::Connected);
				if (conn.getState() == ConnectionState::Disconnecting) {
					ENGINE_LOG("Send MessageType::DISCONNECT to ", connComp.conn->address());
					if (auto msg = conn.beginMessage<MessageType::DISCONNECT>()) {
					}
				}

				if (connComp.disconnectAt <= now) {
					conn.send(socket);
					conn.setKeySend(0);
					conn.setKeyRecv(0);
					conn.setState(ConnectionState::Disconnected);

					ENGINE_LOG("Disconnected ", ent, " ", connComp.conn->address());
					addressToEntity.erase(addressToEntity.find(connComp.conn->address()));
					world.getComponent<ConnectionComponent>(ent).conn.reset(); // Make sure connection is closed now and not later after defered destroy
					world.deferedDestroyEntity(ent);
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

	void NetworkingSystem::connectTo(const Engine::Net::IPv4Address& addr) {
		const auto ent = getOrCreateEntity(addr);
		auto& conn = world.getComponent<ConnectionComponent>(ent).conn;
		conn->setState(ConnectionState::Connecting);

		if (!conn->getKeyRecv()) {
			conn->setKeyRecv(genKey());
		}
		ENGINE_LOG("TRY CONNECT TO: ", addr, " rkey: ", conn->getKeyRecv(), " skey: ",  conn->getKeySend(), " Tick: ", world.getTick());

		if (auto msg = conn->beginMessage<MessageType::CONNECT_REQUEST>()) {
			msg.write(MESSAGE_PADDING_DATA);
			msg.write(conn->getKeyRecv());
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

		world.addComponent<ActionComponent>(ent);
		world.addComponent<MapEditComponent>(ent);
		world.addComponent<CharacterSpellComponent>(ent);
	}

	Engine::ECS::Entity NetworkingSystem::addConnection(const Engine::Net::IPv4Address& addr) {
		auto ent = world.createEntity();
		ENGINE_INFO("Add connection: ", ent, " ", addr, " ", world.hasComponent<PlayerFlag>(ent), " ");
		auto [it, suc] = addressToEntity.emplace(addr, ent);
		auto& connComp = world.addComponent<ConnectionComponent>(ent);
		connComp.conn = std::make_unique<Connection>(addr, now);
		connComp.conn->setState(ConnectionState::Unconnected);
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
	
	void NetworkingSystem::requestDisconnect(const Engine::Net::IPv4Address& addr) {
		const auto ent = getEntity(addr);
		if (ent == Engine::ECS::INVALID_ENTITY) { return; }

		auto& connComp = world.getComponent<ConnectionComponent>(ent);
		connComp.disconnectAt = Engine::Clock::now() + disconnectTime;
		connComp.conn->setState(ConnectionState::Disconnecting);
		ENGINE_INFO("Request disconnect ", addr, " ", ent);
	}

	void NetworkingSystem::dispatchMessage(Engine::ECS::Entity ent, ConnectionComponent& connComp, const Engine::Net::MessageHeader* hdr) {
		auto& from = *connComp.conn;
		constexpr auto msgToStr = [](const Engine::Net::MessageType& type) -> const char* {
			#define X(Name, Side, State) if (type == MessageType::Name) { return #Name; }
			#include <Game/MessageType.xpp>
			return "UNKNOWN";
		};

		switch(hdr->type) {
			#define X(Name, Side, State) case MessageType::Name: {\
				/*ENGINE_LOG("MESSAGE: ", #Name, " ", hdr->seq, " ", hdr->size);/**/\
				handleMessageType<MessageType::Name>(ent, connComp, from, *hdr); break; };
			#include <Game/MessageType.xpp>
			default: {
				ENGINE_WARN("Unhandled network message type ", static_cast<int32>(hdr->type));
			}
		}
		
		const byte* stop = reinterpret_cast<const byte*>(hdr) + sizeof(*hdr) + hdr->size;
		const byte* curr = static_cast<const byte*>(from.read(0));
		const auto rem = stop - curr;

		if (rem > 0) {
			ENGINE_WARN("Incomplete read of network message ", msgToStr(hdr->type), " (", rem, " bytes remaining). Ignoring.");
			from.read(rem);
		} else if (rem < 0) {
			ENGINE_WARN("Read past end of network messge type ", msgToStr(hdr->type)," (", rem, " bytes remaining).");
		}
	}
}
