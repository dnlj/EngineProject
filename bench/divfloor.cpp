// STD
#include <cmath>
#include <cstdlib>
#include <cinttypes>

// Engine
#include <Engine/Zip.hpp>

// Bench
#include <Bench/bench.hpp>
#include <Bench/Dist/Uniform.hpp>


BENCH(divfloor_empty) {
	ctx.startSample();
	for (auto data : dataset) {
		Bench::observe(data);
	}
	ctx.stopSample();
}

BENCH(divfloor_div_mul_tern) {
	ctx.startSample();
	for (auto [num, den] : dataset) {
		const auto q = num / den;
		const auto x = q * den == num ? q : q - (num < 0);
		Bench::observe(x);
	}
	ctx.stopSample();
}

BENCH(divfloor_div_mod_tern) {
	ctx.startSample();
	for (auto [num, den] : dataset) {
		const auto q = num / den;
		const auto r = num % den;
		const auto x = q - (r ? num < 0 : 0);
		Bench::observe(x);
	}
	ctx.stopSample();
}

BENCH(divfloor_div_mod_and) {
	ctx.startSample();
	for (auto [num, den] : dataset) {
		const auto q = num / den;
		const auto r = num % den;
		const auto x = q - (r && num < 0);
		Bench::observe(x);
	}
	ctx.stopSample();
}

BENCH(divfloor_std_tern) {
	ctx.startSample();
	for (auto [num, den] : dataset) {
		const auto d = std::div(num, den);
		const auto x = d.quot - (d.rem ? num < 0 : 0);
		Bench::observe(x);
	}
	ctx.stopSample();
}


namespace {
	// TODO: do these include negatives?
	constexpr auto count = 1234321;
	namespace D = Bench::Dist;
	using UniformI32 = Engine::Zip<
		D::Uniform<int32_t, count, 0>,
		D::Uniform<int32_t, count, 1>
	>;
	using UniformI64 = Engine::Zip<
		D::Uniform<int64_t, count, 0>,
		D::Uniform<int64_t, count, 1>
	>;
}

BENCH_GROUP("divfloor", 1000, 10000);

BENCH_USE(divfloor_empty, UniformI32);
BENCH_USE(divfloor_empty, UniformI64);

BENCH_USE(divfloor_div_mul_tern, UniformI32);
BENCH_USE(divfloor_div_mul_tern, UniformI64);

BENCH_USE(divfloor_div_mod_tern, UniformI32);
BENCH_USE(divfloor_div_mod_tern, UniformI64);

BENCH_USE(divfloor_div_mod_and, UniformI32);
BENCH_USE(divfloor_div_mod_and, UniformI64);

BENCH_USE(divfloor_std_tern, UniformI32);
BENCH_USE(divfloor_std_tern, UniformI64);
