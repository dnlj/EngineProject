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
				constexpr auto s = MapChunk::blockSize;

				mapSystem.setValueAt(
					engine.camera.screenToWorld(engine.inputManager.getMousePosition()) + glm::vec2{0, 0},
					value
				);

				for (int x = -1; x < 2; ++x) {
					for (int y = -1; y < 2; ++y) {
						mapSystem.setValueAt(
							engine.camera.screenToWorld(engine.inputManager.getMousePosition()) + glm::vec2{x * s, y * s},
							value
						);
					}
				}
			};

			virtual void onBindPress() override {
				apply();
			};

			virtual void onBindHold() override {
				apply();
			};
	};
}
