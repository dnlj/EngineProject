#pragma once

// Engine
#include <Engine/Clock.hpp>
#include <Engine/RingBuffer.hpp>

// Game
#include <Game/Common.hpp>


namespace Game {
	class NetworkStatsComponent {
		public:
			//struct Stats {
			//	Engine::Clock::TimePoint time;
			//	struct {
			//		float32 diff = 0;
			//		float32 avg = 0;
			//	} sent, recv;
			//};
			//Engine::RingBuffer<Stats> buffer;
			//uint64 lastTotalBytesSent;
			//uint64 lastTotalBytesRecv;

			int32 inputBufferSize = 0;
			float32 idealInputBufferSize = 0;

			//uint64 displaySentTotal;
			//uint64 displayRecvTotal;
			//float32 displaySentAvg;
			//float32 displayRecvAvg;
			//float32 displayPing;
			//float32 displayJitter;
			//float32 displayLoss;
			//int32 displayInputBufferSize;
			//float32 displayIdealInputBufferSize;
	};
}
