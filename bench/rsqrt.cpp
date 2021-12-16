// Bench
#include <Bench/bench.hpp>
#include <Bench/Dist/Uniform.hpp>


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

namespace {
	using UniformF = Bench::Dist::Uniform<float, 100000>;
	using UniformD = Bench::Dist::Uniform<float, 100000>;
}

BENCH_GROUP("rsqrt");
BENCH_USE(rsqrt_empty, UniformF);
BENCH_USE(rsqrt_empty, UniformD);
BENCH_USE(rsqrt_std, UniformF);
BENCH_USE(rsqrt_std, UniformD);

//BENCH_USE(sqrt_sse, Uniform<float>);
//BENCH_USE(sqrt_sse, Uniform<double>);
