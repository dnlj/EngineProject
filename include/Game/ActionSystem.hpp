#pragma once
// STD
#include <string>

// Engine
#include <Engine/Clock.hpp>
#include <Engine/FlatHashMap.hpp>
#include <Engine/Input/ActionListener.hpp>

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

		public:
			void processAction(Engine::Input::ActionId aid, Engine::Input::Value curr);
			void processAction(Engine::ECS::Entity ent, Engine::Input::ActionId aid, Engine::Input::Value curr);

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
	};
}

#include <Game/ActionSystem.ipp>
