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

	/**
	 * "Optimized reciprocal square root function"
	 * Derived from "A Brief History of InvSqrt" by Matthew Robertson (2012)
	 * @see https://cs.uwaterloo.ca/~m32rober/rsqrt.pdf
	 */
	template<class Float>
	Float mrob_rsqrt(Float number) {
		constexpr bool is32 = std::same_as<Float, float>;
		static_assert(is32 || std::same_as<Float, double>);
		using UInt = std::conditional_t<is32, uint32_t, uint64_t>;
		constexpr UInt magic = is32 ? 0x5f375a86 : 0x5fe6eb50c7b537a9;

		UInt i;
		Float x2, y;
		x2 = number * Float{0.5};
		y = number;
		i = *(UInt *) &y;
		i = magic - (i >> 1);
		y = *(Float *) &i;
		y = y * (Float{1.5} - (x2 * y * y));
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

	template<class D, class F>
	void calcAccuracy(Bench::Context& ctx, const D& dataset, F&& func) {
		std::vector<long double> samples;

		for (auto data : dataset) {
			const auto actual = std_rsqrt<long double>(data);
			const auto result = func(data);
			const auto err = std::abs((result / actual) - 1.0L);
			samples.push_back(err);
		}

		const auto props = Bench::calcSampleProperties(samples);
		ctx.set("E-min", props.min);
		ctx.set("E-max", props.max);
		ctx.set("E-avg", props.mean);
		ctx.set("E-dev", props.stddev);
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
		calcAccuracy(ctx, dataset, std_rsqrt<typename D::ValueType>);
		return;
	}

	ctx.startSample();
	for (auto data : dataset) {
		Bench::observe(std_rsqrt(data));
	}
	ctx.stopSample();
}

BENCH(rsqrt_sse) {
	if constexpr (SinglePass) {
		calcAccuracy(ctx, dataset, sse_rsqrt);
		return;
	}
	ctx.startSample();
	for (auto data : dataset) {
		Bench::observe(sse_rsqrt(data));
	}
	ctx.stopSample();
}

BENCH(rsqrt_sse_newton) {
	if constexpr (SinglePass) {
		calcAccuracy(ctx, dataset, sse_rsqrt_newton);
		return;
	}
	ctx.startSample();
	for (auto data : dataset) {
		Bench::observe(sse_rsqrt_newton(data));
	}
	ctx.stopSample();
}

BENCH(rsqrt_quake) {
	if constexpr (SinglePass) {
		calcAccuracy(ctx, dataset, Q_rsqrt);
		return;
	}
	ctx.startSample();
	for (auto data : dataset) {
		Bench::observe(Q_rsqrt(data));
	}
	ctx.stopSample();
}

BENCH(rsqrt_mrob) {
	if constexpr (SinglePass) {
		calcAccuracy(ctx, dataset, mrob_rsqrt<typename D::ValueType>);
		return;
	}
	ctx.startSample();
	for (auto data : dataset) {
		Bench::observe(mrob_rsqrt(data));
	}
	ctx.stopSample();
}

namespace {
	using UniformF = Bench::Dist::Uniform<float,  1234321>;
	using UniformD = Bench::Dist::Uniform<double, 1234321>;
}

BENCH_GROUP("rsqrt", 1000, 10000);
BENCH_USE(rsqrt_empty, UniformF);
BENCH_USE(rsqrt_empty, UniformD);
BENCH_USE(rsqrt_std, UniformF);
BENCH_USE(rsqrt_std, UniformD);
BENCH_USE(rsqrt_sse, UniformF);
BENCH_USE(rsqrt_sse_newton, UniformF);
BENCH_USE(rsqrt_quake, UniformF);
BENCH_USE(rsqrt_mrob, UniformF);
BENCH_USE(rsqrt_mrob, UniformD);
