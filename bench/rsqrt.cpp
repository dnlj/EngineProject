// Bench
#include <Bench/bench.hpp>
#include <Bench/Dist/Uniform.hpp>

namespace {
	/**
	 * "Fast Inverse Square Root"
	 * Slightly modified from Quake III Areana
	 * @see https://en.wikipedia.org/wiki/Fast_inverse_square_root
	 * @see https://github.com/id-Software/Quake-III-Arena/blob/master/code/game/q_math.c#L552
	 */
	float Q_rsqrt(float number) {
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

	template<class T>
	ENGINE_INLINE T std_rsqrt(const T num) {
		return T{1} / std::sqrt(num);
	}

	ENGINE_INLINE float sse_rsqrt(const float num) {
		return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(num)));
	}

	ENGINE_INLINE float sse_rsqrt_newton(const float num) {
		const auto est = sse_rsqrt(num);
		return est * (1.5f - num * 0.5f * est * est);
	}
}

BENCH(rsqrt_empty) {
	ctx.startSample();
	for (auto data : dataset) {
		Bench::observe(data);
	}
	ctx.stopSample();
}

BENCH(rsqrt_std) {
	if constexpr (SinglePass) {
		for (auto data : dataset) {
			const auto actual = std_rsqrt<long double>(data);
			const auto result = std_rsqrt(data);
			const auto diff = actual - result;
			const auto percent = std::abs(diff / actual);
			// TODO: how do we specify output columns, flat, avg, etc.
			Bench::observe(std_rsqrt(data));
		}
		return;
	}

	ctx.set("foo", 0.5, Bench::StatInterp::Flat);
	ctx.set("bar", "this is a test", Bench::StatInterp::Flat);

	ctx.startSample();
	for (auto data : dataset) {
		Bench::observe(std_rsqrt(data));
	}
	ctx.stopSample();
}

BENCH(rsqrt_sse) {
	ctx.startSample();
	for (auto data : dataset) {
		Bench::observe(sse_rsqrt(data));
	}
	ctx.stopSample();
}

BENCH(rsqrt_sse_newton) {
	ctx.startSample();
	for (auto data : dataset) {
		Bench::observe(sse_rsqrt_newton(data));
	}
	ctx.stopSample();
}

BENCH(rsqrt_quake) {
	ctx.startSample();
	for (auto data : dataset) {
		Bench::observe(Q_rsqrt(data));
	}
	ctx.stopSample();
}

namespace {
	using UniformF = Bench::Dist::Uniform<float,  1234321>;
	using UniformD = Bench::Dist::Uniform<double, 1234321>;
}

BENCH_GROUP("rsqrt");
BENCH_USE(rsqrt_empty, UniformF);
BENCH_USE(rsqrt_empty, UniformD);
BENCH_USE(rsqrt_std, UniformF);
BENCH_USE(rsqrt_std, UniformD);
BENCH_USE(rsqrt_sse, UniformF);
BENCH_USE(rsqrt_sse_newton, UniformF);
BENCH_USE(rsqrt_quake, UniformF);
