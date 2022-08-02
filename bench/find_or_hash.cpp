// STD
#include <unordered_set>

// Bench
#include <Bench/bench.hpp>
#include <Bench/Dist/FixedStrings.hpp>
#include <Bench/Dist/Uniform.hpp>


namespace {
	static pcg32_k16384 rng = pcg_extras::seed_seq_from<std::random_device>{};

	auto resample(auto dist, size_t count) {
		std::vector<std::remove_cvref_t<decltype(*dist.begin())>> out;
		out.reserve(count);
		std::ranges::sample(dist, std::back_inserter(out), count, rng);
		std::ranges::shuffle(out, rng);
		return out;
	}
}

BENCH(linear_subset) {
	const auto find = resample(dataset, dataset.size() / 4);
	const auto end = dataset.end();

	ctx.startSample();
	for (const auto& key : find) {
		Bench::observe(std::ranges::find(dataset, key) == end);
	}
	ctx.stopSample();
}

BENCH(linear_ffo_subset) {
	const auto find = resample(dataset, dataset.size() / 4);
	const auto end = dataset.end();
	const auto fbegin = find.begin();
	const auto fend = find.end();

	ctx.startSample();
	auto curr = dataset.begin();
	while (true) {
		curr = std::find_first_of(curr, end, fbegin, fend);
		Bench::observe(curr == end);
		if (curr == end) { break; }
		++curr;
	}
	ctx.stopSample();
}

BENCH(rhh_hash_subset) {
	using T = std::remove_cvref_t<decltype(*dataset.begin())>;
	robin_hood::unordered_flat_set<T> set;

	for (const auto& key : dataset) {
		set.insert(key);
	}

	const auto find = resample(dataset, dataset.size() / 4);
	const auto end = set.end();

	ctx.startSample();
	for (const auto& key : find) {
		Bench::observe(set.find(key) == end);
	}
	ctx.stopSample();
}

BENCH(std_hash_subset) {
	std::unordered_set<std::remove_cvref_t<decltype(*dataset.begin())>> set;
	for (const auto& key : dataset) {
		set.insert(key);
	}

	const auto find = resample(dataset, dataset.size() / 4);
	const auto end = set.end();

	ctx.startSample();
	for (const auto& key : find) {
		Bench::observe(set.find(key) == end);
	}
	ctx.stopSample();
}

namespace {
	// TODO: small set counts are probably not very accurate since they are so fast. Whats a good way to handle this?

	using Fixed_16_16 = Bench::Dist::FixedStrings<16, 16>;
	using Fixed_16_256 = Bench::Dist::FixedStrings<256, 16>;
	using Fixed_16_1024 = Bench::Dist::FixedStrings<1024, 16>;
	using Fixed_16_4096 = Bench::Dist::FixedStrings<4096, 16>;

	using Fixed_32_16 = Bench::Dist::FixedStrings<16, 32>;
	using Fixed_32_256 = Bench::Dist::FixedStrings<256, 32>;
	using Fixed_32_1024 = Bench::Dist::FixedStrings<1024, 32>;
	using Fixed_32_4096 = Bench::Dist::FixedStrings<4096, 32>;

	using Uniform_16 = Bench::Dist::Uniform<int, 16>;
	using Uniform_32 = Bench::Dist::Uniform<int, 32>;
	using Uniform_128 = Bench::Dist::Uniform<int, 128>;
	using Uniform_256 = Bench::Dist::Uniform<int, 256>;
	using Uniform_1024 = Bench::Dist::Uniform<int, 1024>;
}

BENCH_GROUP("find_or_hash")

BENCH_USE(linear_subset, Fixed_16_16);
BENCH_USE(linear_ffo_subset, Fixed_16_16);
BENCH_USE(std_hash_subset, Fixed_16_16);
BENCH_USE(rhh_hash_subset, Fixed_16_16);
BENCH_USE(linear_subset, Fixed_16_256);
BENCH_USE(linear_ffo_subset, Fixed_16_256);
BENCH_USE(std_hash_subset, Fixed_16_256);
BENCH_USE(rhh_hash_subset, Fixed_16_256);
BENCH_USE(linear_subset, Fixed_16_1024);
BENCH_USE(linear_ffo_subset, Fixed_16_1024);
BENCH_USE(std_hash_subset, Fixed_16_1024);
BENCH_USE(rhh_hash_subset, Fixed_16_1024);
BENCH_USE(linear_subset, Fixed_16_4096);
BENCH_USE(linear_ffo_subset, Fixed_16_4096);
BENCH_USE(std_hash_subset, Fixed_16_4096);
BENCH_USE(rhh_hash_subset, Fixed_16_4096);

BENCH_USE(linear_subset, Fixed_32_16);
BENCH_USE(linear_ffo_subset, Fixed_32_16);
BENCH_USE(std_hash_subset, Fixed_32_16);
BENCH_USE(rhh_hash_subset, Fixed_32_16);
BENCH_USE(linear_subset, Fixed_32_256);
BENCH_USE(linear_ffo_subset, Fixed_32_256);
BENCH_USE(std_hash_subset, Fixed_32_256);
BENCH_USE(rhh_hash_subset, Fixed_32_256);
BENCH_USE(linear_subset, Fixed_32_1024);
BENCH_USE(linear_ffo_subset, Fixed_32_1024);
BENCH_USE(std_hash_subset, Fixed_32_1024);
BENCH_USE(rhh_hash_subset, Fixed_32_1024);
BENCH_USE(linear_subset, Fixed_32_4096);
BENCH_USE(linear_ffo_subset, Fixed_32_4096);
BENCH_USE(std_hash_subset, Fixed_32_4096);
BENCH_USE(rhh_hash_subset, Fixed_32_4096);

BENCH_USE(linear_subset, Uniform_16);
BENCH_USE(linear_ffo_subset, Uniform_16);
BENCH_USE(std_hash_subset, Uniform_16);
BENCH_USE(rhh_hash_subset, Uniform_16);
BENCH_USE(linear_subset, Uniform_32);
BENCH_USE(linear_ffo_subset, Uniform_32);
BENCH_USE(std_hash_subset, Uniform_32);
BENCH_USE(rhh_hash_subset, Uniform_32);
BENCH_USE(linear_subset, Uniform_128);
BENCH_USE(linear_ffo_subset, Uniform_128);
BENCH_USE(std_hash_subset, Uniform_128);
BENCH_USE(rhh_hash_subset, Uniform_128);
BENCH_USE(linear_subset, Uniform_256);
BENCH_USE(linear_ffo_subset, Uniform_256);
BENCH_USE(std_hash_subset, Uniform_256);
BENCH_USE(rhh_hash_subset, Uniform_256);
BENCH_USE(linear_subset, Uniform_1024);
BENCH_USE(linear_ffo_subset, Uniform_1024);
BENCH_USE(std_hash_subset, Uniform_1024);
BENCH_USE(rhh_hash_subset, Uniform_1024);
