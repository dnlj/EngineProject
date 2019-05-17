#pragma once

// STD
#include <vector>

namespace Engine {
	// TODO: move
	class InputListener {
		public:
			// TODO: Doc
			virtual void OnBindPressed() = 0;

			// TODO: Doc
			virtual void OnBindReleased() = 0;

			// TODO: Doc - called every frame between pressed and released
			virtual void OnBindHeld() = 0; // TODO: should this be called on pressed?

			// TODO: Doc - called while the key is down (interval determined by os)
			// TODO: Should we even handle this? Only time it would be useful is UI - which is handled separately.
			//virtual void OnBindRepeat() = 0;
	};

	class Bind {
	};

	class InputManager2 {
		public:
			// addListener();
			// removeListener();
			// TODO: how do binds work for mouse/axis?
			// TODO: per frame event queue? If we are using listeners is this useful?
			
		private:
			std::vector<InputListener> listeners;
	};
}
