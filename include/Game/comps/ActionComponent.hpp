#pragma once

// STD
#include <vector>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Input/Action.hpp>

// Game
#include <Game/Common.hpp>


namespace Game {
	class ActionComponent {
		private:
			std::vector<Engine::Input::Action> actions;

		public:
			void grow(Engine::Input::ActionId size);

			/**
			 * Gets the action associated with an id.
			 */
			Engine::Input::Action& get(Engine::Input::ActionId aid);

			/**
			 * Gets the value of the an action by id.
			 */
			Engine::Input::Value getValue(Engine::Input::ActionId aid);

			/**
			 * Gets the value of an action by id.
			 * @tparam T The type to interpret the value as.
			 */
			template<class T>
			T getValue(Engine::Input::ActionId aid);

			/**
			 * Gets the value of multiple actions.
			 */
			template<class T, class... ActionIdN>
			auto getValue(Engine::Input::ActionId aid1, Engine::Input::ActionId aid2, ActionIdN... aidN);

			/** @see getValue */
			template<class T, class... ActionIdN, int32... Is>
			auto getValue(const std::tuple<ActionIdN...>& aidN, std::index_sequence<Is...>);
			
			/** @see getValue */
			template<class T, class... ActionIdN>
			auto getValue(const std::tuple<ActionIdN...>& aidN);
			
			/** @see getValue */
			template<class T, int32 N, int32... Is>
			auto getValue(const std::array<Engine::Input::ActionId, N>& aidN, std::index_sequence<Is...>);
			
			/** @see getValue */
			template<class T, int32 N>
			auto getValue(const std::array<Engine::Input::ActionId, N>& aidN);
			
			/** @see getValue */
			template<class T, int32 N, int32... Is>
			auto getValue(const Engine::Input::ActionId (&ids)[N], std::index_sequence<Is...>);
			
			/** @see getValue */
			template<class T, int32 N>
			auto getValue(const Engine::Input::ActionId (&ids)[N]);
	};
}

#include <Game/comps/ActionComponent.ipp>
