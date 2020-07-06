#pragma once

// Engine
#include <Engine/EngineInstance.hpp>
#include <Engine/ECS/Entity.hpp>
#include <Engine/Net/Replication.hpp>
#include <Engine/Net/PacketReader.hpp>
#include <Engine/Net/PacketWriter.hpp>


namespace Engine::ECS {
	template<class World>
	class NetworkComponent {
		public:
			constexpr static auto netRepl() { return Engine::Net::Replication::NONE; };
			
			void netTo(Engine::Net::PacketWriter& writer) const {}
			void netToInit(EngineInstance& engine, World& world, Entity ent, Net::PacketWriter& writer) const {};
			
			void netFrom(Engine::Net::PacketReader& reader) {}
			void netFromInit(EngineInstance& engine, World& world, Entity ent, Net::PacketReader& reader) {};
	};
}
