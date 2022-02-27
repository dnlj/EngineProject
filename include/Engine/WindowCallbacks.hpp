#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Input/InputEvent.hpp>


namespace Engine {
	class WindowCallbackFunctions {
		public:
			virtual ~WindowCallbackFunctions() {};

			/** Called when system settings are changed */
			virtual void settingsChanged() = 0;

			virtual void resizeCallback(int32 w, int32 h) = 0;
			virtual void keyCallback(Engine::Input::InputEvent event) = 0;
			virtual void charCallback(wchar_t character) = 0;
			virtual void mouseButtonCallback(Engine::Input::InputEvent event) = 0;
			virtual void mouseWheelCallback(Engine::Input::InputEvent event) = 0;
			virtual void mouseMoveCallback(Engine::Input::InputEvent event) = 0;
			virtual void mouseLeaveCallback() = 0;
			virtual void mouseEnterCallback() = 0;
	};

	template<class UserData>
	class WindowCallbacks : public WindowCallbackFunctions {
		public:
			UserData* userdata;
	};
}
