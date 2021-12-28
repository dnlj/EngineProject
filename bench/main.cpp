// STD
#include <algorithm>
#include <numeric>

// Engine
#include <Engine/Clock.hpp>
#include <Engine/CommandLine/Parser.hpp>

// Bench
#include <Bench/bench.hpp>
#include <Bench/Dist/ZipIterator.hpp>

#include "noise.hpp"


int main(int argc, char* argv[]) {
	// start /wait /b /realtime Bench.exe -g=rsqrt
	#ifdef ENGINE_OS_WINDOWS
		SetConsoleOutputCP(CP_UTF8);

		if (auto console = GetStdHandle(STD_OUTPUT_HANDLE);
			!console || !SetConsoleMode(console, ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
			ENGINE_WARN("Unable to configure windows terminal input.");
		}
	#endif

	Engine::CommandLine::Parser parser; parser
		.add<std::string>("group", 'g', {}, "Which groups to run.", false)
		.add<std::string>("format", 'f', "table", "TODO: other output formats"); // TODO: table, json, csv, html, etc.

	parser.parse(argc, argv);

	// TODO: columns: name | dataset | avg | std dev | min | max | % baseline | iters / sec | custom1 | custom2 | ....
	//       cont. iters will need to also consider dataset size
	// TODO: user defined output columns
	// TODO: instead of specifying iterations should we specify a time? "run each case for 5 seconds" and then we would have iters/time instead of time/iter. pros? cons?
	//       cont. or go until we hit a certain confidence threshhold?
	// TODO: need to store both wall time and cpu time

	// TODO: rm - testing
	std::vector<float> foo = {0.1f, 0.2f, 3.0f, 4.5f};
	std::vector<int> bar = {4, 3, 2, 1};
	std::vector<double> baz = {9.8, 7.6, 6.5, 5.4};
	Bench::Dist::Zip a{foo, bar, baz};
	for (auto [i,j,k] : a) {
		ENGINE_LOG("i,j,k = ", i, ", ", j, ", ", k);
	}

	auto& ctx = Bench::Context::instance();

	if (auto format = parser.get<std::string>("format")) {
		if (*format != "table") { ENGINE_WARN("Unknown output format"); }
	}

	if (auto group = parser.get<std::string>("group")) {
		ctx.runGroup(*group);
	}

	/*
	std::vector<Engine::Clock::Duration> times;
	times.resize(10);

	for (auto& t : times) {
		t = noise();
		std::cout << "Time: " << Seconds{t}.count() << "\n";
	}

	Engine::Clock::Duration sum = std::accumulate(times.cbegin(), times.cend(), Engine::Clock::Duration{});
	std::cout << "Avg: " << Seconds{sum / times.size()}.count() << "s\n";
	*/

	std::cout.flush();
	std::cin.get();
	return 0;
}
