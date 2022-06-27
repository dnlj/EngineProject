#pragma once

// Game
#include <Game/UI/AutoList.hpp>


namespace Game::UI {
	class InfoPane : public AutoList {
		private:
			EUI::Button* disconnect;

			struct Sample {
				Engine::Clock::TimePoint time;
				uint64 frames;
			};

			Engine::Clock::TimePoint lastUpdate;
			Engine::RingBuffer<Sample> fpsSamples;
			
		public:
			enum {
				FPS,
				Tick,
				TickScale,
			};

			InfoPane(EUI::Context* context);
			void update();
	};
}
