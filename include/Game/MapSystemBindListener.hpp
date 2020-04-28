#pragma once

// Engine
#include <Engine/Glue/glm.hpp>
#include <Engine/Input/ActionListener.hpp>

// Game
#include <Game/MapSystem.hpp>


namespace Game {
	template<int value>
	class MapSystemBindListener {
		public:
			MapSystemBindListener(MapSystem& mapSystem, Engine::EngineInstance& engine)
				: mapSystem{mapSystem}
				, engine{engine}
				, targetIds{engine.actionManager.getId("Target_X", "Target_Y")} {
			};

			bool operator()(Engine::Input::Value curr, Engine::Input::Value prev) {
				if (curr && !prev) {
					apply();
				}
				return false;
			};

		private:
			MapSystem& mapSystem;
			Engine::EngineInstance& engine;
			std::array<Engine::Input::ActionId, 2> targetIds;

			void apply() {
				constexpr auto s = MapChunk::blockSize;
				const auto TODO_rm = engine.actionManager.getValue<float32>(targetIds);
				const auto mousePos = engine.camera.screenToWorld(engine.actionManager.getValue<float32>(targetIds));

				std::cout << "apply: " << TODO_rm.x << ", " << TODO_rm.y << "\n";

				mapSystem.setValueAt(
					mousePos + glm::vec2{0, 0},
					value
				);

				for (int x = -1; x < 2; ++x) {
					for (int y = -1; y < 2; ++y) {
						mapSystem.setValueAt(
							mousePos + glm::vec2{x * s, y * s},
							value
						);
					}
				}
			};
	};
}
