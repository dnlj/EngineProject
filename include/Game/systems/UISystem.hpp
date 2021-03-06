#pragma once

// STD
#include <cmath>

// Engine
#include <Engine/Input/ActionId.hpp>
#include <Engine/StaticRingBuffer.hpp>
#include <Engine/ImGui/ImGui.hpp>

// Game
#include <Game/System.hpp>

namespace Game {
	class UISystem : public System {
		public:
			UISystem(SystemArg arg);

			void setup();
			void run(float32 dt);
			void tick();

		private:
			std::stringstream ss;

			Engine::Clock::TimePoint now;
			Engine::Clock::TimePoint lastUpdate;
			Engine::Clock::Duration updateRate = std::chrono::milliseconds{100};
			bool update = false;

			Engine::Clock::TimePoint rollingWindow;
			Engine::Clock::Duration rollingWindowSize = std::chrono::milliseconds{10'000};

			// TODO: rm RollingData for consistency with above
			struct FrameData {
				float32 dt = 0.0f;
			};
			Engine::RingBuffer<std::pair<FrameData, Engine::Clock::TimePoint>> frameData;

			Engine::Clock::Duration fpsAvgWindow = std::chrono::milliseconds{500};
			float32 fps = 0.0f;

			void ui_connect();
			void ui_debug();
			void ui_coordinates();
			void ui_render();
			void ui_camera();
			void ui_netsim();
			void ui_network();
			void ui_nethealth();
			void ui_entities();

			template<bool B>
			static ImPlotPoint netGetPointAvg(void* data, int idx);

			template<bool B>
			static ImPlotPoint netGetDiff(void* data, int idx);
	};
}
