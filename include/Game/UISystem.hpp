#pragma once

// STD
#include <cmath>

// Engine
#include <Engine/ECS/EntityFilter.hpp>
#include <Engine/Input/ActionId.hpp>
#include <Engine/StaticRingBuffer.hpp>

// Game
#include <Game/System.hpp>

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
			Engine::Clock::Duration rollingWindowSize = std::chrono::milliseconds{5000};

			// TODO: rm RollingData for consistency with above
			struct FrameData {
				float32 dt = 0.0f;

				uint64 sent = 0;
				float32 sentDiff = NAN;

				uint64 recv = 0;
				float32 recvDiff = NAN;
			};
			Engine::RingBuffer<std::pair<FrameData, Engine::Clock::TimePoint>> frameData;

			Engine::Clock::Duration fpsAvgWindow = std::chrono::milliseconds{500};
			float32 fps = 0.0f;

			void ui_connect();
			void ui_debug();
			void ui_coordinates();

			void ui_network();

			std::array<Engine::Input::ActionId, 2> targetIds;
			Engine::ECS::EntityFilter& connFilter;
	};
}
