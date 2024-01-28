#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Input/InputEvent.hpp>
#include <Engine/Input/KeyCode.hpp>


namespace Engine {
	class WindowCallbacks {
		public:
			virtual ~WindowCallbacks() {};

			/** Called when system settings are changed */
			virtual void settingsChanged() = 0;

			virtual void resizeCallback(int32 w, int32 h) = 0;
			virtual void keyCallback(Input::InputEvent event) = 0;
			virtual void charCallback(wchar_t character, Input::KeyCode code) = 0;
			virtual void mouseButtonCallback(Input::InputEvent event) = 0;
			virtual void mouseWheelCallback(Input::InputEvent event) = 0;
			virtual void mouseMoveCallback(Input::InputEvent event) = 0;
			virtual void mouseLeaveCallback() = 0;
			virtual void mouseEnterCallback() = 0;
			virtual void gainFocus() = 0;
			virtual void loseFocus() = 0;
	};
}
