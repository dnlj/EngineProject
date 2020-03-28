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
#include <Engine/Input/Bind.hpp>
#include <Engine/Input/InputId.hpp>
#include <Engine/Input/InputBindMapping.hpp>
#include <Engine/Input/InputState.hpp>
#include <Engine/Input/InputSequence.hpp>

// TODO: move
namespace Engine::Input {
	// TODO: change button version to use string view?
	using AxisId = int8;

	class AxisBind {
		public:
			AxisBind(std::string name) : name{name} {}
			const std::string name;
			float32 value = 0.0f;
			// TODO: listeners
	};
}

// TODO: cleanup docs
namespace Engine::Input {
	class InputManager {
		public:
			/**
			 * Updates the hold status of all binds.
			 * Should be called once per frame.
			 */
			void update();

			// TODO: doc
			void processInput(const InputState& is);

			// TODO: doc
			void processAxisInput(const InputState& is);

			// TODO: doc
			void processButtonInput(const InputState& is);
			
			/**
			 * Creats a bind with the name @p name.
			 * @param name The name of the bind.
			 * @return The BindId of the bind.
			 */
			BindId createButtonBind(std::string name);
			
			/**
			 * Gets the bind id for the bind @p name.
			 * @param name The name of the bind.
			 * @return The BindId for the given name.
			 */
			BindId getButtonBindId(const std::string& name) const;
			
			/**
			 * Gets the Bind associated with @p name.
			 * @param name The name of the bind.
			 * @return The Bind for the given name.
			 */
			Bind& getButtonBind(const std::string& name);
			
			/**
			 * Gets the Bind associated with @p bid.
			 * @param bid The bind id.
			 * @return The Bind for the given id.
			 */
			Bind& getButtonBind(const BindId bid);
			
			/**
			 * Adds a mapping from the input sequence @p inputs to the bind @p name.
			 * @param name The name of the bind.
			 * @param inputs The sequence of inputs to add for the bind.
			 */
			void addButtonMapping(const std::string& name, InputSequence inputs);

			/**
			 * Creates a new bind with the given name.
			 * @return The id of the new bind.
			 */
			AxisId createAxisBind(std::string_view name);


			/**
			 * Gets the id associated with a name.
			 */
			AxisId getAxisId(std::string_view name) const;

			/**
			 * Gets the ids associated with multiple names.
			 */
			template<class StringView1, class StringView2, class... StringViewN>
			auto getAxisId(StringView1 view1, StringView2 view2, StringViewN... viewN) const {
				return std::tuple{getAxisId(view1), getAxisId(view2), getAxisId(viewN)...};
			}
			
			/**
			 * Gets the bind associated with a name.
			 */
			AxisBind& getAxisBind(const AxisId aid);

			/**
			 * Gets the bind associated with a name.
			 */
			AxisBind& getAxisBind(std::string_view name);

			/**
			 * Associates a name with an input axis.
			 */
			void addAxisMapping(std::string_view name, InputId axis);

			/**
			 * Gets the value of an axis.
			 */
			float32 getAxisValue(AxisId aid);
			
			/**
			 * Gets the value of multiple axes.
			 */
			template<class... AxisIdN>
			auto getAxisValue(AxisId aid1, AxisId aid2, AxisIdN... aidN) {
				return glm::vec<2 + sizeof...(AxisIdN), float32, glm::defaultp>{
					getAxisValue(aid1), getAxisValue(aid2), getAxisValue(aidN),
				};
			}

			/** @see getAxisValue */
			template<class... AxisIdN, int32... Is>
			auto getAxisValue(const std::tuple<AxisIdN...>& aidN, std::index_sequence<Is...>) {
				return glm::vec<sizeof...(Is), float32, glm::defaultp>{
					getAxisValue(std::get<Is>(aidN))...
				};
			}

			/** @see getAxisValue */
			template<class... AxisIdN>
			auto getAxisValue(const std::tuple<AxisIdN...>& aidN) {
				return getAxisValue(aidN, std::make_index_sequence<sizeof...(AxisIdN)>{});
			}
			
			/** @see getAxisValue */
			template<int32 N, int32... Is>
			auto getAxisValue(const AxisId (&ids)[N], std::index_sequence<Is...>) {
				return glm::vec<N, float32, glm::defaultp>{getAxisValue(ids[Is])...};
			};
			
			/** @see getAxisValue */
			template<int32 N>
			auto getAxisValue(const AxisId (&ids)[N]) {
				return getAxisValue(ids, std::make_index_sequence<N>{});
			};

		private:
			/** Stores a set of indices into #inputBindMappings where each index corresponds to an InputBindMapping that uses the given InputId. */
			FlatHashMap<InputId, std::vector<uint16_t>, Hash<InputId>> buttonToMapping;

			/** Stores every InputBindMapping used by this manager. */
			std::vector<InputBindMapping> buttonMappings;

			/** Stores every Bind used by this manager. */
			std::vector<Bind> buttonBinds;
			
			/** Stores every axis id associated with a input axis. */
			FlatHashMap<InputId, std::vector<AxisId>, Hash<InputId>> axisMappings;

			/** Stores every axis bind used by this manager. */
			std::vector<AxisBind> axisBinds;
	};
}
