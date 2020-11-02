#pragma once
// STD
#include <string>

// Engine
#include <Engine/Clock.hpp>
#include <Engine/FlatHashMap.hpp>
#include <Engine/Input/ActionListener.hpp>
#include <Engine/Net/Connection.hpp>

// Game
#include <Game/System.hpp>
#include <Game/comps/ActionComponent.hpp>
#include <Game/Connection.hpp>

namespace Game {
	class ActionSystem : public System {
		private:
			std::vector<std::vector<Engine::Input::ActionListener>> actionIdToListeners;

		public:
			ActionSystem(SystemArg arg);

			void preTick();
			void tick();

			void updateButtonState(Button btn, bool val);
			void updateButtonState(Engine::ECS::Entity ent, Button btn, bool val);

			void updateAxisState(Axis axis, float32 val);
			void updateAxisState(Engine::ECS::Entity ent, Axis axis, float32 val);

			template<class Listener>
			void addListener(Engine::Input::ActionId aid, Listener&& listener);

			Engine::Input::ActionId count() const;

		public:
			void recvActions(Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity fromEnt);
			void recvActionsClient(Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity fromEnt);
			void recvActionsServer(Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity fromEnt);
	};
}

#include <Game/systems/ActionSystem.ipp>
