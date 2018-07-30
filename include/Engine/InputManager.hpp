#pragma once

// STD
#include <string>
#include <unordered_map>

namespace Engine {
	// TODO: Doc
	// TODO: Axis/mouse support.
	class InputManager {
		public:
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
			
			/**
			 * Checks if the bind associated with @p name is pressed.
			 * @param[in] name The name of the bind.
			 * @return True if the bind is pressed; otherwise false.
			 */
			bool isPressed(const std::string& name) const;
			
			/**
			 * Checks if the bind associated with @p name was released this update.
			 * @param[in] name The name of the bind.
			 * @return True if the bind was released this update; otherwise false.
			 */
			bool wasReleased(const std::string& name) const;

			/**
			 * Maps the bind @p name with the scancode @p code.
			 * @param[in] name The name of the bind.
			 * @param[in] code The scancode.
			 */
			void bind(std::string name, ScanCode code);

			/**
			 * Signals a new update. This should be called once per frame.
			 */
			void update();

			/**
			 * The callback for updating keys.
			 * @param[in] code The scancode.
			 * @param[in] action The action. See GLFW documentation for more information.
			 */
			void callback(ScanCode code, int action);

		private:
			/** Maps binds to scancodes */
			std::unordered_map<std::string, ScanCode> binds;

			/** The state of the input last update */
			std::unordered_map<ScanCode, bool> previousState;
			
			/** The state of the input this update */
			std::unordered_map<ScanCode, bool> currentState;
	};
}
