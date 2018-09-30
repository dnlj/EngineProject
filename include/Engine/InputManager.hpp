#pragma once

// GLM
#include <glm/common.hpp>

// STD
#include <string>
#include <unordered_map>

namespace Engine {
	// TODO: Doc
	// TODO: Axis/mouse support.
	class InputManager {
		public:
			/** TODO: Doc */
			using BindID = uint32_t;

			/** The type of scancodes */
			using ScanCode = int;

			/**
			 * Constructor.
			 */
			InputManager();

			/**
			 * Checks if the bind associated with @p name was pressed this update.
			 * @param[in] name The name of the bind.
			 * @return True if the bind was pressed this update; otherwise false.
			 */
			bool wasPressed(const std::string& name) const;

			// TODO: Doc
			bool wasPressed(BindID bid) const;
			
			/**
			 * Checks if the bind associated with @p name is pressed.
			 * @param[in] name The name of the bind.
			 * @return True if the bind is pressed; otherwise false.
			 */
			bool isPressed(const std::string& name) const;

			// TODO: Doc
			bool isPressed(BindID bid) const;
			
			/**
			 * Checks if the bind associated with @p name was released this update.
			 * @param[in] name The name of the bind.
			 * @return True if the bind was released this update; otherwise false.
			 */
			bool wasReleased(const std::string& name) const;

			// TODO: Doc
			bool wasReleased(BindID bid) const;

			// TODO: Reverse arg order
			/**
			 * Maps the bind @p name with the scancode @p code.
			 * @param[in] name The name of the bind.
			 * @param[in] code The scancode.
			 */
			void bind(const std::string& name, ScanCode code);

			// TODO: Add BindID version of bind

			/**
			 * Gets the current position of the mouse.
			 * @return The x and y position of the mouse.
			 */
			glm::vec2 getMousePosition() const;

			/**
			 * Signals a new update. This should be called once per frame.
			 */
			void update();

			/**
			 * The callback for updating keys.
			 * @param[in] code The scancode.
			 * @param[in] action The action. See GLFW documentation for more information.
			 */
			void keyCallback(ScanCode code, int action);

			/**
			 * The callback for updating the mouse position
			 * @param[in] x The x position of the mouse.
			 * @param[in] y The y position of the mouse.
			 */
			void mouseCallback(double x, double y);

		private:
			// TODO: Doc
			BindID nextBindID = 0;

			/** Maps bind names to bind ids */
			std::unordered_map<std::string, BindID> bindToBindID;

			// TODO: Doc
			std::vector<BindID> scanCodeToBindID;

			// TODO: Doc
			std::vector<uint8_t> currentState;

			// TODO: Doc
			std::vector<uint8_t> previousState;

			/** The current position of the mouse */
			glm::vec2 mousePosition;
			 
	};
}
