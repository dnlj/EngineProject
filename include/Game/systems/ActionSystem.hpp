#pragma once
// STD
#include <string>

// Game
#include <Game/System.hpp>
#include <Game/comps/ActionComponent.hpp>
#include <Game/Connection.hpp>

namespace Game {
	class ActionSystem : public System {
		public:
			ActionSystem(SystemArg arg);

			void setup();

			void preTick();
			void tick();

			void updateActionState(Action act, bool val);
			void updateActionState(Engine::ECS::Entity ent, Action act, bool val);

			void updateTarget(glm::vec2 val);
			void updateTarget(Engine::ECS::Entity ent, glm::vec2 val);

		public:
			void recvActionsClient(Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity fromEnt, Engine::Net::BufferReader& msg);
			void recvActionsServer(Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity fromEnt, Engine::Net::BufferReader& msg);
	};
}
