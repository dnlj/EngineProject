#pragma once

// GLM
#include <glm/common.hpp>

// STD
#include <string>
#include <array>
#include <unordered_map>


namespace Engine {
	/** The type to use for bind ids */
	using BindId = int;

	/** The states for bind events */
	// TODO: Move
	enum class BindState : uint8_t {
		PRESS,
		RELEASE,
		INVALID,
	};

	// TODO: Doc
	// TODO: Move
	class BindEvent {
		public:
			BindId bid;
			BindState state;
	};

	// TODO: Doc
	// TODO: Move
	// TODO: Make a fixed_queue class
	class BindEventQueue {
		public:
			std::array<BindEvent, 32> events;
			int size;
	};

	// TODO: Doc
	// TODO: Axis/mouse support.
	class InputManager {
		public:
			/** The type of scancodes */
			using ScanCode = int;

			// TODO: Doc
			using MouseButton = int;

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

			/** @see wasPressed */
			bool wasPressed(BindId bid) const;
			
			/**
			 * Checks if the bind associated with @p name is pressed.
			 * @param[in] name The name of the bind.
			 * @return True if the bind is pressed; otherwise false.
			 */
			bool isPressed(const std::string& name) const;

			/** @see isPressed */
			bool isPressed(BindId bid) const;
			
			/**
			 * Checks if the bind associated with @p name was released this update.
			 * @param[in] name The name of the bind.
			 * @return True if the bind was released this update; otherwise false.
			 */
			bool wasReleased(const std::string& name) const;

			/** @see wasReleased */
			bool wasReleased(BindId bid) const;

			/**
			 * Binds a key to a bind name.
			 * @param[in] code The scan code of the key.
			 * @param[in] name The name of the bind.
			 */
			void bindkey(ScanCode code, const std::string& name);

			/**
			 * Binds a mouse button to a bind name.
			 * @param[in] button The mouse button to bind.
			 * @param[in] name The name of the bind
			 */
			void bindMouseButton(MouseButton button, const std::string& name);

			/**
			 * Gets the current position of the mouse.
			 * Origin is top left
			 * @return The x and y position of the mouse.
			 */
			glm::vec2 getMousePosition() const;

			/**
			 * Signals a new update. This should be called once per frame.
			 */
			void update();

			/**
			 * @return The queue of bind events since the last update.
			 */
			const BindEventQueue& getBindEventQueue() const;

			/**
			 * Get the id of a bind name.
			 * @param[in] name The name of the bind.
			 * @return The id of the bind.
			 */
			BindId getBindId(const std::string& name);

			/**
			 * The callback for updating keys.
			 * See GLFW documentation for more information.
			 * @param[in] code The scancode.
			 * @param[in] action The action.
			 */
			void keyCallback(ScanCode code, int action);

			/**
			 * The callback for updating the mouse position.
			 * See GLFW documentation for more information.
			 * @param[in] x The x position of the mouse.
			 * @param[in] y The y position of the mouse.
			 */
			void mouseCallback(double x, double y);

			/**
			 * The callback for updating the mouse buttons.
			 * See GLFW documentation for more information.
			 * @param[in] button The mouse button.
			 * @param[in] action The action.
			 */
			void mouseCallback(MouseButton button, int action);

		private:
			/** The next id to assign to bind */
			BindId nextBindID = 0;

			/** Maps bind names to bind ids */
			std::unordered_map<std::string, BindId> bindToBindID;

			/** Converts from scan codes to bind ids */
			std::vector<BindId> scanCodeToBindID;

			// TODO: Doc
			std::vector<BindId> mouseButtonToBindId;

			/** The current state of binds */
			std::vector<uint8_t> currentState;

			/** The previous state of binds */
			std::vector<uint8_t> previousState;

			/** The current position of the mouse */
			glm::vec2 mousePosition;

			/** The queue for bind events since last update */
			BindEventQueue bindEventQueue;

			// TODO: Doc
			void insertBindEvent(BindEvent event);
	};
}
