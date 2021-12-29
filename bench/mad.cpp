// STD
#include <cmath>

// Engine
#include <Engine/Zip.hpp>

// Bench
#include <Bench/bench.hpp>
#include <Bench/Dist/Uniform.hpp>


BENCH(mad_empty) {
	ctx.startSample();
	for (auto data : dataset) {
		Bench::observe(data);
	}
	ctx.stopSample();
}

BENCH(mad_naive) {
	ctx.startSample();
	for (auto [a,b,c] : dataset) {
		Bench::observe(a * b + c);
	}
	ctx.stopSample();
}

BENCH(mad_std) {
	ctx.startSample();
	for (auto [a,b,c] : dataset) {
		Bench::observe(std::fma(a,b,c));
	}
	ctx.stopSample();
}


namespace {
	constexpr auto count = 1234321;
	namespace D = Bench::Dist;
	using UniformF = Engine::Zip<
		D::Uniform<float, count, 0>,
		D::Uniform<float, count, 1>,
		D::Uniform<float, count, 2>
	>;
	using UniformD = Engine::Zip<
		D::Uniform<double, count, 0>,
		D::Uniform<double, count, 1>,
		D::Uniform<double, count, 2>
	>;
	using UniformI32 = Engine::Zip<
		D::Uniform<int32_t, count, 0>,
		D::Uniform<int32_t, count, 1>,
		D::Uniform<int32_t, count, 2>
	>;
	using UniformI64 = Engine::Zip<
		D::Uniform<int64_t, count, 0>,
		D::Uniform<int64_t, count, 1>,
		D::Uniform<int64_t, count, 2>
	>;
}

BENCH_GROUP("mad");

BENCH_USE(mad_empty, UniformF);
BENCH_USE(mad_empty, UniformD);
BENCH_USE(mad_empty, UniformI32);
BENCH_USE(mad_empty, UniformI64);

BENCH_USE(mad_naive, UniformF);
BENCH_USE(mad_naive, UniformD);
BENCH_USE(mad_naive, UniformI32);
BENCH_USE(mad_naive, UniformI64);

BENCH_USE(mad_std, UniformF);
BENCH_USE(mad_std, UniformD);
BENCH_USE(mad_std, UniformI32);
BENCH_USE(mad_std, UniformI64);
