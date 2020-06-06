#pragma once

// STD
#include <cmath>

// Engine
#include <Engine/Input/ActionId.hpp>
#include <Engine/StaticRingBuffer.hpp>
#include <Engine/ImGui/ImGui.hpp>

// Game
#include <Game/System.hpp>
#include <Game/EntityFilter.hpp>

namespace Game {
	class UISystem : public System {
		public:
			UISystem(SystemArg arg);

			void setup();
			void run(float32 dt);
			void tick(float32 dt);

		private:
			Engine::Clock::TimePoint now;
			Engine::Clock::TimePoint lastUpdate;
			Engine::Clock::Duration updateRate = std::chrono::milliseconds{100};
			bool update = false;

			Engine::Clock::TimePoint rollingWindow;
			Engine::Clock::Duration rollingWindowSize = std::chrono::milliseconds{5'000};

			// TODO: rm RollingData for consistency with above
			struct FrameData {
				float32 dt = 0.0f;

				struct {
					uint64 total = 0;
					float32 diff = NAN;
					float32 avg = NAN;
				} netstats[2];
			};
			Engine::RingBuffer<std::pair<FrameData, Engine::Clock::TimePoint>> frameData;

			Engine::Clock::Duration fpsAvgWindow = std::chrono::milliseconds{500};
			float32 fps = 0.0f;

			void ui_connect();
			void ui_debug();
			void ui_coordinates();

			void ui_network();

			template<int32 I>
			static ImVec2 netGetPointAvg(void* data, int idx);

			template<int32 I>
			static ImVec2 netGetDiff(void* data, int idx);

			std::array<Engine::Input::ActionId, 2> targetIds;
			EntityFilter& connFilter;
			EntityFilter& activePlayerFilter;
	};
}
