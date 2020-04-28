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
		public:
			ActionId create(const std::string& name);
			ActionId getId(const std::string& name);

			// TODO: split
			template<class... String>
			auto getId(const std::string& name1, const std::string& name2, String&&... nameN) {
				return std::array<ActionId, 2 + sizeof...(String)>{
					getId(name1), getId(name2), getId(std::forward<String>(nameN)) ...
				};
			}

			Action& get(const std::string& name);
			Action& get(ActionId aid);

			Value getValue(const std::string& name);
			Value getValue(ActionId aid);

			template<class T>
			T getValue(ActionId aid) {
				return *reinterpret_cast<T*>(&getValue(aid));
			}
			
			// TODO: split
			template<class T, class... ActionIdN>
			auto getValue(ActionId aid1, ActionId aid2, ActionIdN... aidN) {
				return glm::vec<2 + sizeof...(ActionIdN), T, glm::defaultp>{
					getValue<T>(aid1), getValue<T>(aid2), getValue<T>(aidN), ...
				};
			}
			
			// TODO: split
			template<class T, class... ActionIdN, int32... Is>
			auto getValue(const std::tuple<ActionIdN...>& aidN, std::index_sequence<Is...>) {
				return glm::vec<sizeof...(Is), T, glm::defaultp>{
					getValue<T>(std::get<Is>(aidN))...
				};
			}
			
			// TODO: split
			template<class T, class... ActionIdN>
			auto getValue(const std::tuple<ActionIdN...>& aidN) {
				return getValue<T>(aidN, std::make_index_sequence<sizeof...(ActionIdN)>{});
			}
			
			// TODO: split
			template<class T, int32 N, int32... Is>
			auto getValue(const std::array<ActionId, N>& aidN, std::index_sequence<Is...>) {
				return glm::vec<N, T, glm::defaultp>{getValue<T>(aidN[Is])...};
			}
			
			// TODO: split
			template<class T, int32 N>
			auto getValue(const std::array<ActionId, N>& aidN) {
				return getValue<T>(aidN, std::make_index_sequence<N>{});
			}

			// TODO: split
			template<class T, int32 N, int32... Is>
			auto getValue(const ActionId (&ids)[N], std::index_sequence<Is...>) {
				return glm::vec<N, T, glm::defaultp>{getValue<T>(ids[Is])...};
			};
			
			// TODO: split
			template<class T, int32 N>
			auto getValue(const ActionId (&ids)[N]) {
				return getValue<T>(ids, std::make_index_sequence<N>{});
			};

			// TODO: back to forward ref
			void addListener(ActionId aid, ActionListener listener);


		private:
			Engine::FlatHashMap<std::string, ActionId> actionNameToId;
			std::vector<Action> actions;
	};
}

#include <Engine/Input/ActionManager.ipp>
