#include <Engine/Clock.hpp>
#include <Engine/Noise/WorleyNoise.hpp>

Engine::Clock::Duration noise() {
	Engine::Noise::WorleyNoise noise{1645448290048005};

	constexpr int w = 1024;
	constexpr int h = w;

	auto data = new float[w][h];

	const auto start = Engine::Clock::now();
	for (int x = 0; x < w; ++x) {
		for (int y = 0; y < h; ++y) {
			auto a = noise.valueF2F1(static_cast<float>(x), static_cast<float>(y));
			data[x][y] = a.value;
		}
	}
	const auto stop = Engine::Clock::now();
	delete[] data;
	return stop - start;
}
