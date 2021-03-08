#pragma once

// Engine
#include <Engine/Clock.hpp>
#include <Engine/ImGui/ImGui.hpp>


namespace Game {
	class MapTestUI {
		private:
			Engine::Clock::TimePoint startTime;
			Engine::Clock::TimePoint stopTime;
		public:
			MapTestUI() = default;

			void generate() {
				startTime = Engine::Clock::now();
				stopTime = Engine::Clock::now();
			}

			void render() {
				// TODO: imgui
				if (!ImGui::Begin("Map Playground", nullptr, ImGuiWindowFlags_None)) { ImGui::End(); return; }
				ImGui::Text("Test!");
				ImGui::End();
			}
	};
}
