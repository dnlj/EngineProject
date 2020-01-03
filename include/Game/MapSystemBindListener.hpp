#pragma once

// Engine
#include <Engine/Input/BindPressListener.hpp>

// Game
#include <Game/MapSystem.hpp>


namespace Game {
	template<int value>
	class MapSystemBindListener : public Engine::Input::BindPressListener, public Engine::Input::BindHoldListener {
		public:
			MapSystemBindListener(MapSystem& mapSystem, Engine::EngineInstance& engine)
				: mapSystem{mapSystem}
				, engine{engine} {
			};

		private:
			MapSystem& mapSystem;
			Engine::EngineInstance& engine;
			void apply() {
				mapSystem.setValueAt(
					engine.camera.screenToWorld(engine.inputManager.getMousePosition()),
					value
				);
			};

			virtual void onBindPress() override {
				apply();
			};

			virtual void onBindHold() override {
				apply();
			};
	};
}
