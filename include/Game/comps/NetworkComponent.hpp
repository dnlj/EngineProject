#pragma once

// Engine
#include <Engine/EngineInstance.hpp>
#include <Engine/Net/Replication.hpp>
#include <Engine/ECS/Entity.hpp>

// Game
#include <Game/Connection.hpp>


namespace Game {
	class World;
	class NetworkComponent {
		public:
			constexpr static auto netRepl() { return Engine::Net::Replication::NONE; };

			void netTo(Connection& conn) const {}
			void netToInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Connection& conn) const {};
			
			void netFrom(Connection& reader) {}
			void netFromInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Connection& conn) {};
	};
}
