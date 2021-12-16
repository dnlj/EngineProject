#include <algorithm>
#include <numeric>

#include <Engine/Clock.hpp>
#include <Bench/bench.hpp>
#include <Engine/CommandLine/Parser.hpp>

#include "noise.hpp"

// TODO: count template param?
template<class T>
struct Uniform {
	std::vector<T> stuff;

	Uniform() {
		for (T i = 0; i < 100000; ++i) {
			stuff.push_back(i);
		}
	}

	decltype(auto) begin() const { return stuff.begin(); }
	decltype(auto) end() const { return stuff.end(); }
};

BENCH(rsqrt_empty) {
	ctx.startSample();
	for (auto data : dataset) {
		Bench::observe(data);
	}
	ctx.stopSample();
}

BENCH(rsqrt_std) {
	// TODO: where to put this doc?
	// We actually want to sample the whole dataset at once
	// because the cost of `Clock::now()` could actually out weigh our operation we are benchmarking.
	// For example, on MSVC `now` performs: 2x call, 4x idiv, 2x imul, 8x other
	// whereas just doing there vector iteration is just an: add, cmp, jne
	// So timing the loop as a whole should give us more accurate numbers.

	ctx.startSample();
	for (auto data : dataset) {
		Bench::observe(1 / std::sqrt(data));
	}
	ctx.stopSample();
}

BENCH_GROUP("rsqrt");
BENCH_USE(rsqrt_empty, Uniform<float>);
BENCH_USE(rsqrt_std, Uniform<float>);
BENCH_USE(rsqrt_std, Uniform<double>);
BENCH_USE(rsqrt_std, Uniform<long double>);

//BENCH_USE(sqrt_sse, Uniform<float>);
//BENCH_USE(sqrt_sse, Uniform<double>);


int main(int argc, char* argv[]) {
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

	using Seconds = std::chrono::duration<long double, std::ratio<1, 1>>;
	using Milliseconds = std::chrono::duration<long double, std::milli>;

	// TODO: columns: name | dataset | avg | std dev | min | max | % baseline | iters / sec | custom1 | custom2 | ....
	//       cont. iters will need to also consider dataset size
	// TODO: user defined output columns
	// TODO: instead of specifying iterations should we specify a time? "run each case for 5 seconds" and then we would have iters/time instead of time/iter. pros? cons?
	//       cont. or go until we hit a certain confidence threshhold?
	// TODO: need to store both wall time and cpu time

	auto& ctx = Bench::Context::instance();

	if (auto format = parser.get<std::string>("format")) {
		if (*format != "table") { ENGINE_WARN("Unknown output format"); }
	}

	if (auto group = parser.get<std::string>("group")) {
		ctx.runGroup(*group);
	}

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
