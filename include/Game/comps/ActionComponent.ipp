#pragma once

// Game
#include <Game/comps/ActionComponent.hpp>


namespace Game {
	template<class T>
	T ActionComponent::getValue(Engine::Input::ActionId aid) {
		return *reinterpret_cast<T*>(&getValue(aid));
	}
	
	template<class T, class... ActionIdN>
	auto ActionComponent::getValue(Engine::Input::ActionId aid1, Engine::Input::ActionId aid2, ActionIdN... aidN) {
		return glm::vec<2 + sizeof...(ActionIdN), T, glm::defaultp>{
			getValue<T>(aid1), getValue<T>(aid2), getValue<T>(aidN), ...
		};
	}
	
	template<class T, class... ActionIdN, int32... Is>
	auto ActionComponent::getValue(const std::tuple<ActionIdN...>& aidN, std::index_sequence<Is...>) {
		return glm::vec<sizeof...(Is), T, glm::defaultp>{
			getValue<T>(std::get<Is>(aidN))...
		};
	}
	
	template<class T, class... ActionIdN>
	auto ActionComponent::getValue(const std::tuple<ActionIdN...>& aidN) {
		return getValue<T>(aidN, std::make_index_sequence<sizeof...(ActionIdN)>{});
	}
	
	template<class T, int32 N, int32... Is>
	auto ActionComponent::getValue(const std::array<Engine::Input::ActionId, N>& aidN, std::index_sequence<Is...>) {
		return glm::vec<N, T, glm::defaultp>{getValue<T>(aidN[Is])...};
	}
	
	template<class T, int32 N>
	auto ActionComponent::getValue(const std::array<Engine::Input::ActionId, N>& aidN) {
		return getValue<T>(aidN, std::make_index_sequence<N>{});
	}

	template<class T, int32 N, int32... Is>
	auto ActionComponent::getValue(const Engine::Input::ActionId (&ids)[N], std::index_sequence<Is...>) {
		return glm::vec<N, T, glm::defaultp>{getValue<T>(ids[Is])...};
	};
	
	template<class T, int32 N>
	auto ActionComponent::getValue(const Engine::Input::ActionId (&ids)[N]) {
		return getValue<T>(ids, std::make_index_sequence<N>{});
	};
}
