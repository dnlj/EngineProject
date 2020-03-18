#pragma once

// STD
#include <chrono>

// Engine
#include <Engine/Engine.hpp>


namespace Engine {
	class Clock {
		public:
			using Seconds = std::chrono::duration<float32, std::ratio<1, 1>>;
			using Nanoseconds = std::chrono::duration<int64, std::nano>;
			
			// For std compatibility
			using rep = int64;
			using period = std::nano;
			using duration = Nanoseconds;
			using time_point = std::chrono::time_point<Clock>;
			constexpr static bool is_steady = true;

			using Rep = rep;
			using Period = period;
			using Duration = duration;
			using TimePoint = time_point;
			constexpr static bool isSteady = is_steady;

			static time_point now() noexcept {
				using highres = std::chrono::high_resolution_clock;
				static_assert(std::is_same_v<highres::period, period>);
				return time_point{highres::now().time_since_epoch()};
			};
	};
}
