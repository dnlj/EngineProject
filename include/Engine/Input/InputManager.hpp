#pragma once

// STD
#include <vector>
#include <string>
#include <string_view>

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

			// TODO: remove? Represent mouse/axis as a bind?
			/**
			 * Gets the current position of the mouse.
			 * Origin is top left
			 * @return The x and y position of the mouse.
			 */
			glm::vec2 getMousePosition() const;

			// TODO: remove? Represent mouse/axis as a bind?
			void mouseCallback(int16 axis, int32 value);

			AxisId createAxisBind(std::string_view name);
			AxisId getAxisId(std::string_view name) const;
			AxisBind& getAxisBind(const AxisId aid);
			AxisBind& getAxisBind(std::string_view name);
			void addAxisMapping(std::string_view name, InputId axis);
			float32 getAxisValue(AxisId aid);

		private:
			/** Stores a set of indices into #inputBindMappings where each index corresponds to an InputBindMapping that uses the given InputId. */
			FlatHashMap<InputId, std::vector<uint16_t>, Hash<InputId>> buttonToMapping;

			/** Stores every InputBindMapping used by this manager. */
			std::vector<InputBindMapping> buttonMappings;

			/** Stores every Bind used by this manager. */
			std::vector<Bind> buttonBinds;

			/** Stores the current position of the mouse. */
			glm::vec2 mousePosition; // TODO: remove? Represent mouse/axis as a bind?

			
			// TODO: doc
			FlatHashMap<InputId, std::vector<AxisId>, Hash<InputId>> axisMappings;
			std::vector<AxisBind> axisBinds;
	};
}
