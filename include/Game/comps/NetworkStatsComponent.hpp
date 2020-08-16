#pragma once

// Engine
#include <Engine/Clock.hpp>
#include <Engine/StaticRingBuffer.hpp>

// Game
#include <Game/Common.hpp>


namespace Game {
	class NetworkStatsComponent {
		public:
			struct Stats {
				Engine::Clock::TimePoint time;
				struct {
					float32 diff = 0;
					float32 avg = 0;
				} sent, recv;
			};
			Engine::RingBuffer<Stats> buffer;
			uint64 lastTotalBytesSent;
			uint64 lastTotalBytesRecv;
	};
}
