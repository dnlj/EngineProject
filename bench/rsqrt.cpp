// Bench
#include <Bench/bench.hpp>
#include <Bench/Dist/Uniform.hpp>


/**
 * "Fast Inverse Square Root"
 * Slightly modified from Quake III Areana
 * @see https://en.wikipedia.org/wiki/Fast_inverse_square_root
 * @see https://github.com/id-Software/Quake-III-Arena/blob/master/code/game/q_math.c#L552
 */
static float Q_rsqrt(float number) {
	long i;
	float x2, y;
	const float threehalfs = 1.5f;

	x2 = number * 0.5f;
	y  = number;
	i  = * ( long * ) &y;
	i  = 0x5f3759df - ( i >> 1 );
	y  = * ( float * ) &i;
	y  = y * ( threehalfs - ( x2 * y * y ) ); // 1st iteration

	return y;
}

// TODO: custom output column - accuracy

BENCH(rsqrt_empty) {
	ctx.startSample();
	for (auto data : dataset) {
		Bench::observe(data);
	}
	ctx.stopSample();
}

BENCH(rsqrt_std) {
	ctx.startSample();
	for (auto data : dataset) {
		Bench::observe(1 / std::sqrt(data));
	}
	ctx.stopSample();
}

BENCH(rsqrt_sse) {
	ctx.startSample();
	for (auto data : dataset) {
		static_assert(std::same_as<decltype(data), float>);
		Bench::observe(_mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(data))));
	}
	ctx.stopSample();
}

BENCH(rsqrt_sse_newton) {
	ctx.startSample();
	for (auto data : dataset) {
		static_assert(std::same_as<decltype(data), float>);
		const auto est = _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(data)));
		Bench::observe(est * (1.5f - data * 0.5f * est * est));
	}
	ctx.stopSample();
}

BENCH(rsqrt_quake) {
	ctx.startSample();
	for (auto data : dataset) {
		static_assert(std::same_as<decltype(data), float>);
		Bench::observe(Q_rsqrt(data));
	}
	ctx.stopSample();
}

namespace {
	using UniformF = Bench::Dist::Uniform<float,  1235321>;
	using UniformD = Bench::Dist::Uniform<double, 1235321>;
}

BENCH_GROUP("rsqrt");
BENCH_USE(rsqrt_empty, UniformF);
BENCH_USE(rsqrt_empty, UniformD);
BENCH_USE(rsqrt_std, UniformF);
BENCH_USE(rsqrt_std, UniformD);
BENCH_USE(rsqrt_sse, UniformF);
BENCH_USE(rsqrt_sse_newton, UniformF);
BENCH_USE(rsqrt_quake, UniformF);
