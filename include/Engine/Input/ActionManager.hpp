#pragma once

// STD
#include <string>
#include <vector>
#include <functional>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/FlatHashMap.hpp>
#include <Engine/Input/Action.hpp>


namespace Engine::Input {

	class ActionManager {
		private:
			Engine::FlatHashMap<std::string, ActionId> actionNameToId;
			std::vector<Action> actions;

		public:
			/**
			 * Creates a new action with a name.
			 */
			ActionId create(const std::string& name);

			/**
			 * Gets the id of an action.
			 */
			ActionId getId(const std::string& name);

			/**
			 * Gets the id of multiple actions.
			 */
			template<class... String>
			auto getId(const std::string& name1, const std::string& name2, String&&... nameN);

			/**
			 * Gets the action associated with a name.
			 */
			Action& get(const std::string& name);

			/**
			 * Gets the action associated with an id.
			 */
			Action& get(ActionId aid);

			/**
			 * Gets the value of the an action by name.
			 */
			Value getValue(const std::string& name);

			/**
			 * Gets the value of the an action by id.
			 */
			Value getValue(ActionId aid);

			/**
			 * Gets the value of an action by id.
			 * @tparam T The type to interpret the value as.
			 */
			template<class T>
			T getValue(ActionId aid);

			/**
			 * Gets the value of multiple actions.
			 */
			template<class T, class... ActionIdN>
			auto getValue(ActionId aid1, ActionId aid2, ActionIdN... aidN);

			/** @see getValue */
			template<class T, class... ActionIdN, int32... Is>
			auto getValue(const std::tuple<ActionIdN...>& aidN, std::index_sequence<Is...>);
			
			/** @see getValue */
			template<class T, class... ActionIdN>
			auto getValue(const std::tuple<ActionIdN...>& aidN);
			
			/** @see getValue */
			template<class T, int32 N, int32... Is>
			auto getValue(const std::array<ActionId, N>& aidN, std::index_sequence<Is...>);
			
			/** @see getValue */
			template<class T, int32 N>
			auto getValue(const std::array<ActionId, N>& aidN);
			
			/** @see getValue */
			template<class T, int32 N, int32... Is>
			auto getValue(const ActionId (&ids)[N], std::index_sequence<Is...>);
			
			/** @see getValue */
			template<class T, int32 N>
			auto getValue(const ActionId (&ids)[N]);

			template<class Listener>
			void addListener(ActionId aid, Listener&& listener);
	};
}

#include <Engine/Input/ActionManager.ipp>
