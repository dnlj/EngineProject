#include <algorithm>
#include <numeric>

#include <Engine/Clock.hpp>
#include <Bench/bench.hpp>

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

BENCH(rsqrt_std) {
	puts("TEST1");
	// TODO: where to put this doc?
	// We actually want to sample the whole dataset at once
	// because the cost of `Clock::now()` could actually out weigh our operation we are benchmarking.
	// For example, on MSVC `now` performs: 2x call, 4x idiv, 2x imul, 8x other
	// whereas just doing there vector iteration is just an: add, cmp, jne
	// So timing the loop as a whole should give us more accurate numbers.
	for (auto data : dataset) {
		ctx.startSample(); // TODO: would be nice to have this done automatically, maybe inject in the iterators?
		Bench::observe(1 / std::sqrt(data));
		ctx.stopSample();
		ENGINE_LOG("Time: ", (ctx.sampleStop - ctx.sampleStart).count());
	}
	puts("TEST2");
}

BENCH_GROUP("rsqrt");
BENCH_USE(rsqrt_std, Uniform<float>);
BENCH_USE(rsqrt_std, Uniform<double>);
BENCH_USE_GROUP("rsqrt", rsqrt_std, Uniform<long double>);

//BENCH_USE(sqrt_sse, Uniform<float>);
//BENCH_USE(sqrt_sse, Uniform<double>);


// TODO: something like:
/*

===========================================================
= Group Name
-----------------------------------------------------------
Name      | Dataset         | Avg      | Std Dev | 
-----------------------------------------------------------
sqrt      | Uniform<int>    | 123.45ms | 5.3ms   | ....
sqrt      | Uniform<float>  | 123.45ms | 5.3ms   | ....
sqrt      | Uniform<double> | 123.45ms | 5.3ms   | ....
sqrt_sse  | Uniform<float>  | 123.45ms | 5.3ms   | ....
sqrt_sse  | Uniform<double> | 123.45ms | 5.3ms   | ....
sqrt_cast | Uniform<float>  | 123.45ms | 5.3ms   | ....

*/


// TODO: use engine::commandline
int main(int argc, char* argv[]) {
	using Seconds = std::chrono::duration<long double, std::ratio<1, 1>>;
	using Milliseconds = std::chrono::duration<long double, std::milli>;

	// TODO: print CPU and memory info (model, speed, etc)
	// TODO: columns: name | dataset | avg | std dev | min | max | % baseline | iters / sec | custom1 | custom2 | ....
	//       cont. iters will need to also consider dataset size
	// TODO: user defined output columns
	// TODO: instead of specifying iterations should we specify a time? "run each case for 5 seconds" and then we would have iters/time instead of time/iter. pros? cons?
	//       cont. or go until we hit a certain confidence threshhold?
	// TODO: each group should be able to specify different input data sets to operate on
	// TODO: need to store both wall time and cpu time

	auto& group = Bench::Context::instance().getGroup("rsqrt");
	for (auto& b : group.benchmarks) {
		ENGINE_LOG(">>>>>>>>>>>>>>>>>>>>>> ", b.first);
		b.second.func();
	}
	ENGINE_LOG("<<<<<<<<<<<<<<<<<<<<<<<<");

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
