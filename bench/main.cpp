#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>

#include <Engine/Clock.hpp>

#include "noise.hpp"

int main(int argc, char* argv[]) {
	using Seconds = std::chrono::duration<long double, std::ratio<1, 1>>;
	using Milliseconds = std::chrono::duration<long double, std::milli>;

	std::vector<Engine::Clock::Duration> times;
	times.resize(10);

	for (auto& t : times) {
		t = noise();
		std::cout << "Time: " << Seconds{t}.count() << "\n";
	}

	Engine::Clock::Duration sum = std::accumulate(times.cbegin(), times.cend(), Engine::Clock::Duration{});
	std::cout << "Avg: " << Seconds{sum / times.size()}.count() << "s\n";


	std::cin.get();
	return 0;
}
