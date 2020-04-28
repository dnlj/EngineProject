#pragma once

// STD
#include <vector>
#include <string>
#include <string_view>
#include <type_traits>
#include <tuple>

// GLM
#include <glm/vec2.hpp>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/FlatHashMap.hpp>
#include <Engine/Input/InputId.hpp>
#include <Engine/Input/Bind.hpp>
#include <Engine/Input/InputState.hpp>
#include <Engine/Input/InputSequence.hpp>
#include <Engine/Input/ActionId.hpp>


namespace Engine::Input {
	class InputManager {
		public:
			// TODO: doc
			void processInput(const InputState& is);

			///**
			// * Creates a new bind with the given name.
			// * @return The id of the new bind.
			// */
			//AxisId createAxisBind(std::string_view name);
			//
			///**
			// * Gets the id associated with a name.
			// */
			//AxisId getAxisId(std::string_view name) const;
			//
			///**
			// * Gets the ids associated with multiple names.
			// */
			//template<class StringView1, class StringView2, class... StringViewN>
			//auto getAxisId(StringView1 view1, StringView2 view2, StringViewN... viewN) const {
			//	return std::tuple{getAxisId(view1), getAxisId(view2), getAxisId(viewN)...};
			//}
			//
			///**
			// * Gets the bind associated with a name.
			// */
			//AxisBind& getAxisBind(const AxisId aid);
			//
			///**
			// * Gets the bind associated with a name.
			// */
			//AxisBind& getAxisBind(std::string_view name);
			//
			///**
			// * Associates a name with an input axis.
			// */
			//void addAxisMapping(std::string_view name, InputId axis);
			//
			///**
			// * Gets the value of an axis.
			// */
			//float32 getAxisValue(AxisId aid);
			//
			///**
			// * Gets the value of multiple axes.
			// */
			//template<class... AxisIdN>
			//auto getAxisValue(AxisId aid1, AxisId aid2, AxisIdN... aidN) {
			//	return glm::vec<2 + sizeof...(AxisIdN), float32, glm::defaultp>{
			//		getAxisValue(aid1), getAxisValue(aid2), getAxisValue(aidN),
			//	};
			//}
			//
			///** @see getAxisValue */
			//template<class... AxisIdN, int32... Is>
			//auto getAxisValue(const std::tuple<AxisIdN...>& aidN, std::index_sequence<Is...>) {
			//	return glm::vec<sizeof...(Is), float32, glm::defaultp>{
			//		getAxisValue(std::get<Is>(aidN))...
			//	};
			//}
			//
			///** @see getAxisValue */
			//template<class... AxisIdN>
			//auto getAxisValue(const std::tuple<AxisIdN...>& aidN) {
			//	return getAxisValue(aidN, std::make_index_sequence<sizeof...(AxisIdN)>{});
			//}
			//
			///** @see getAxisValue */
			//template<int32 N, int32... Is>
			//auto getAxisValue(const AxisId (&ids)[N], std::index_sequence<Is...>) {
			//	return glm::vec<N, float32, glm::defaultp>{getAxisValue(ids[Is])...};
			//};
			//
			///** @see getAxisValue */
			//template<int32 N>
			//auto getAxisValue(const AxisId (&ids)[N]) {
			//	return getAxisValue(ids, std::make_index_sequence<N>{});
			//};


			///////////////////////////////////////////////////////////////////////////////////////////////////////////
			// TODO: rm old api
			// TODO: make private:

			// TODO: i dont think we actually need separate axis/button bindings
			enum class BindId : uint16;

			FlatHashMap<InputId, std::vector<BindId>> bindLookup;
			std::vector<Bind> binds;

			template<class Listener>
			BindId addBind(const InputSequence& inputs, Listener&& listener);
			///////////////////////////////////////////////////////////////////////////////////////////////////////////
	};
}

#include <Engine/Input/InputManager.ipp>
