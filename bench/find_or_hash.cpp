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

BENCH(linear_subset_manual) {
	const auto find = resample(dataset, dataset.size() / 4);
	const auto end = dataset.end();

	ctx.startSample();
	for (const auto& key : find) {
		auto found = false;
		for (const auto d : dataset) {
			if (d == key) {
				found = true;
				break;
			}
		}
		Bench::observe(found);
	}
	ctx.stopSample();
}

BENCH(binary_sorted) {
	const auto find = resample(dataset, dataset.size() / 4);
	auto dataset_real = std::vector(dataset.begin(), dataset.end());
	std::ranges::sort(dataset_real);
	const auto end = dataset_real.end();

	ctx.startSample();
	for (const auto& key : find) {
		auto found = std::ranges::lower_bound(dataset_real, key);
		Bench::observe(found == end);
	}
	ctx.stopSample();
}

BENCH(binary_sorted_stack) {
	using T = std::decay_t<decltype(*dataset.begin())>;
	const auto find = resample(dataset, dataset.size() / 4);
	std::array<T, 256> dataset_real;

	assert(std::cmp_less_equal(dataset.size(), dataset_real.size()));
	std::copy(dataset.begin(), dataset.end(), dataset_real.begin());
	std::ranges::sort(dataset_real);

	const auto end = dataset_real.end();

	ctx.startSample();
	for (const auto& key : find) {
		auto found = std::ranges::lower_bound(dataset_real, key);
		Bench::observe(found == end);
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
	namespace D = Bench::Dist;

	using Fixed_16_16 = Bench::Dist::FixedStrings<16, 16, D::Bool::False>;
	using Fixed_16_256 = Bench::Dist::FixedStrings<256, 16, D::Bool::False>;
	using Fixed_16_1024 = Bench::Dist::FixedStrings<1024, 16, D::Bool::False>;
	using Fixed_16_4096 = Bench::Dist::FixedStrings<4096, 16, D::Bool::False>;

	using Fixed_32_16 = Bench::Dist::FixedStrings<16, 32, D::Bool::False>;
	using Fixed_32_256 = Bench::Dist::FixedStrings<256, 32, D::Bool::False>;
	using Fixed_32_1024 = Bench::Dist::FixedStrings<1024, 32, D::Bool::False>;
	using Fixed_32_4096 = Bench::Dist::FixedStrings<4096, 32, D::Bool::False>;

	using Uniform_16 = Bench::Dist::Uniform<int, 16>;
	using Uniform_32 = Bench::Dist::Uniform<int, 32>;
	using Uniform_128 = Bench::Dist::Uniform<int, 128>;
	using Uniform_256 = Bench::Dist::Uniform<int, 256>;
	using Uniform_1024 = Bench::Dist::Uniform<int, 1024>;
}

BENCH_GROUP("find_or_hash", 100, 1000)

BENCH_USE(linear_subset, Fixed_16_16);
BENCH_USE(linear_subset_manual, Fixed_16_16);
BENCH_USE(linear_ffo_subset, Fixed_16_16);
BENCH_USE(std_hash_subset, Fixed_16_16);
BENCH_USE(rhh_hash_subset, Fixed_16_16);
BENCH_USE(binary_sorted, Fixed_16_16);
BENCH_USE(binary_sorted_stack, Fixed_16_16);
BENCH_USE(linear_subset, Fixed_16_256);
BENCH_USE(linear_subset_manual, Fixed_16_256);
BENCH_USE(linear_ffo_subset, Fixed_16_256);
BENCH_USE(std_hash_subset, Fixed_16_256);
BENCH_USE(rhh_hash_subset, Fixed_16_256);
BENCH_USE(binary_sorted, Fixed_16_256);
BENCH_USE(binary_sorted_stack, Fixed_16_256);
BENCH_USE(linear_subset, Fixed_16_1024);
BENCH_USE(linear_subset_manual, Fixed_16_1024);
BENCH_USE(linear_ffo_subset, Fixed_16_1024);
BENCH_USE(std_hash_subset, Fixed_16_1024);
BENCH_USE(rhh_hash_subset, Fixed_16_1024);
BENCH_USE(binary_sorted, Fixed_16_1024);
BENCH_USE(linear_subset, Fixed_16_4096);
BENCH_USE(linear_subset_manual, Fixed_16_4096);
BENCH_USE(linear_ffo_subset, Fixed_16_4096);
BENCH_USE(std_hash_subset, Fixed_16_4096);
BENCH_USE(rhh_hash_subset, Fixed_16_4096);
BENCH_USE(binary_sorted, Fixed_16_4096);

BENCH_USE(linear_subset, Fixed_32_16);
BENCH_USE(linear_subset_manual, Fixed_32_16);
BENCH_USE(linear_ffo_subset, Fixed_32_16);
BENCH_USE(std_hash_subset, Fixed_32_16);
BENCH_USE(rhh_hash_subset, Fixed_32_16);
BENCH_USE(binary_sorted, Fixed_32_16);
BENCH_USE(binary_sorted_stack, Fixed_32_16);
BENCH_USE(linear_subset, Fixed_32_256);
BENCH_USE(linear_subset_manual, Fixed_32_256);
BENCH_USE(linear_ffo_subset, Fixed_32_256);
BENCH_USE(std_hash_subset, Fixed_32_256);
BENCH_USE(rhh_hash_subset, Fixed_32_256);
BENCH_USE(binary_sorted, Fixed_32_256);
BENCH_USE(binary_sorted_stack, Fixed_32_256);
BENCH_USE(linear_subset, Fixed_32_1024);
BENCH_USE(linear_subset_manual, Fixed_32_1024);
BENCH_USE(linear_ffo_subset, Fixed_32_1024);
BENCH_USE(std_hash_subset, Fixed_32_1024);
BENCH_USE(rhh_hash_subset, Fixed_32_1024);
BENCH_USE(binary_sorted, Fixed_32_1024);
BENCH_USE(linear_subset, Fixed_32_4096);
BENCH_USE(linear_subset_manual, Fixed_32_4096);
BENCH_USE(linear_ffo_subset, Fixed_32_4096);
BENCH_USE(std_hash_subset, Fixed_32_4096);
BENCH_USE(rhh_hash_subset, Fixed_32_4096);
BENCH_USE(binary_sorted, Fixed_32_4096);

BENCH_USE(linear_subset, Uniform_16);
BENCH_USE(linear_subset_manual, Uniform_16);
BENCH_USE(linear_ffo_subset, Uniform_16);
BENCH_USE(std_hash_subset, Uniform_16);
BENCH_USE(rhh_hash_subset, Uniform_16);
BENCH_USE(binary_sorted, Uniform_16);
BENCH_USE(binary_sorted_stack, Uniform_16);
BENCH_USE(linear_subset, Uniform_32);
BENCH_USE(linear_subset_manual, Uniform_32);
BENCH_USE(linear_ffo_subset, Uniform_32);
BENCH_USE(std_hash_subset, Uniform_32);
BENCH_USE(rhh_hash_subset, Uniform_32);
BENCH_USE(binary_sorted, Uniform_32);
BENCH_USE(binary_sorted_stack, Uniform_32);
BENCH_USE(linear_subset, Uniform_128);
BENCH_USE(linear_subset_manual, Uniform_128);
BENCH_USE(linear_ffo_subset, Uniform_128);
BENCH_USE(std_hash_subset, Uniform_128);
BENCH_USE(rhh_hash_subset, Uniform_128);
BENCH_USE(binary_sorted, Uniform_128);
BENCH_USE(binary_sorted_stack, Uniform_128);
BENCH_USE(linear_subset, Uniform_256);
BENCH_USE(linear_subset_manual, Uniform_256);
BENCH_USE(linear_ffo_subset, Uniform_256);
BENCH_USE(std_hash_subset, Uniform_256);
BENCH_USE(rhh_hash_subset, Uniform_256);
BENCH_USE(binary_sorted, Uniform_256);
BENCH_USE(binary_sorted_stack, Uniform_256);
BENCH_USE(linear_subset, Uniform_1024);
BENCH_USE(linear_subset_manual, Uniform_1024);
BENCH_USE(linear_ffo_subset, Uniform_1024);
BENCH_USE(std_hash_subset, Uniform_1024);
BENCH_USE(rhh_hash_subset, Uniform_1024);
BENCH_USE(binary_sorted, Uniform_1024);
