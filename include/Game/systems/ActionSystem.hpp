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
#include <Game/EntityFilter.hpp>

namespace Game {
	class ActionSystem : public System {
		private:
			Engine::FlatHashMap<std::string, Engine::Input::ActionId> actionNameToId;
			std::vector<std::vector<Engine::Input::ActionListener>> actionIdToListeners;
			EntityFilter& actionFilter;
			EntityFilter& connFilter;

		public:
			ActionSystem(SystemArg arg);
			void tick(float32 dt);

			void queueAction(Engine::Input::ActionId aid, Engine::Input::Value curr);
			void queueAction(Engine::ECS::Entity ent, Engine::Input::ActionId aid, Engine::Input::Value curr, Engine::ECS::Tick tick);

			/**
			 * Creates a new action with a name.
			 */
			Engine::Input::ActionId create(const std::string& name);

			/**
			 * Gets the id of an action.
			 */
			Engine::Input::ActionId getId(const std::string& name);

			/**
			 * Gets the id of multiple actions.
			 */
			template<class... String>
			auto getId(const std::string& name1, const std::string& name2, String&&... nameN);

			template<class Listener>
			void addListener(Engine::Input::ActionId aid, Listener&& listener);

			Engine::Input::ActionId count() const;

		public:
			void sendActions();
			void recvActions(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity fromEnt);
			void recvActionsClient(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity fromEnt);
			void recvActionsServer(Engine::Net::Connection& from, const Engine::Net::MessageHeader& head, Engine::ECS::Entity fromEnt);
	};
}

#include <Game/systems/ActionSystem.ipp>
