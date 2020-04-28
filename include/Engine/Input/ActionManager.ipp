#pragma once

// Engine
#include <Engine/Input/ActionManager.hpp>


namespace Engine::Input {
	template<class... String>
	auto ActionManager::getId(const std::string& name1, const std::string& name2, String&&... nameN) {
		return std::array<ActionId, 2 + sizeof...(String)>{
			getId(name1), getId(name2), getId(std::forward<String>(nameN)) ...
		};
	}

	template<class T>
	T ActionManager::getValue(ActionId aid) {
		return *reinterpret_cast<T*>(&getValue(aid));
	}
	
	template<class T, class... ActionIdN>
	auto ActionManager::getValue(ActionId aid1, ActionId aid2, ActionIdN... aidN) {
		return glm::vec<2 + sizeof...(ActionIdN), T, glm::defaultp>{
			getValue<T>(aid1), getValue<T>(aid2), getValue<T>(aidN), ...
		};
	}
	
	template<class T, class... ActionIdN, int32... Is>
	auto ActionManager::getValue(const std::tuple<ActionIdN...>& aidN, std::index_sequence<Is...>) {
		return glm::vec<sizeof...(Is), T, glm::defaultp>{
			getValue<T>(std::get<Is>(aidN))...
		};
	}
	
	template<class T, class... ActionIdN>
	auto ActionManager::getValue(const std::tuple<ActionIdN...>& aidN) {
		return getValue<T>(aidN, std::make_index_sequence<sizeof...(ActionIdN)>{});
	}
	
	template<class T, int32 N, int32... Is>
	auto ActionManager::getValue(const std::array<ActionId, N>& aidN, std::index_sequence<Is...>) {
		return glm::vec<N, T, glm::defaultp>{getValue<T>(aidN[Is])...};
	}
	
	template<class T, int32 N>
	auto ActionManager::getValue(const std::array<ActionId, N>& aidN) {
		return getValue<T>(aidN, std::make_index_sequence<N>{});
	}

	template<class T, int32 N, int32... Is>
	auto ActionManager::getValue(const ActionId (&ids)[N], std::index_sequence<Is...>) {
		return glm::vec<N, T, glm::defaultp>{getValue<T>(ids[Is])...};
	};
	
	template<class T, int32 N>
	auto ActionManager::getValue(const ActionId (&ids)[N]) {
		return getValue<T>(ids, std::make_index_sequence<N>{});
	};
	template<class Listener>
	void ActionManager::addListener(ActionId aid, Listener&& listener) {
		actions[aid].listeners.push_back(std::forward<Listener>(listener));
	}
}
