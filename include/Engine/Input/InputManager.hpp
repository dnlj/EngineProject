#pragma once

// STD
#include <vector>
#include <string>

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


namespace Engine::Input {
	class InputManager {
		public:
			/**
			 * Updates the hold status of all binds.
			 * Should be called once per frame.
			 */
			void update();
			
			/**
			 * Updates all binds based on @p is.
			 * @param is The input state that should be used for updating.
			 */
			void processInput(const InputState& is);
			
			/**
			 * Creats a bind with the name @p name.
			 * @param name The name of the bind.
			 * @return The BindId of the bind.
			 */
			BindId createBind(std::string name);
			
			/**
			 * Gets the bind id for the bind @p name.
			 * @param name The name of the bind.
			 * @return The BindId for the given name.
			 */
			BindId getBindId(const std::string& name) const;
			
			/**
			 * Gets the Bind associated with @p name.
			 * @param name The name of the bind.
			 * @return The Bind for the given name.
			 */
			Bind& getBind(const std::string& name);
			
			/**
			 * Gets the Bind associated with @p bid.
			 * @param bid The bind id.
			 * @return The Bind for the given id.
			 */
			Bind& getBind(const BindId bid);
			
			/**
			 * Adds a mapping from the input sequence @p inputs to the bind @p name.
			 * @param name The name of the bind.
			 * @param inputs The sequence of inputs to add for the bind.
			 */
			void addInputBindMapping(const std::string& name, InputSequence inputs);

			// TODO: remove? Represent mouse/axis as a bind?
			/**
			 * Gets the current position of the mouse.
			 * Origin is top left
			 * @return The x and y position of the mouse.
			 */
			glm::vec2 getMousePosition() const;

			// TODO: remove? Represent mouse/axis as a bind?
			void mouseCallback(int16 axis, int32 value);

		private:
			/** Stores a set of indices into #inputBindMappings where each index corresponds to an InputBindMapping that uses the given InputId. */
			FlatHashMap<InputId, std::vector<uint16_t>, Hash<InputId>> inputToMapping;

			/** Stores every InputBindMapping used by this manager. */
			std::vector<InputBindMapping> inputBindMappings;

			/** Stores every Bind used by this manager. */
			std::vector<Bind> binds;

			/** Stores the current position of the mouse. */
			glm::vec2 mousePosition; // TODO: remove? Represent mouse/axis as a bind?
	};
}
