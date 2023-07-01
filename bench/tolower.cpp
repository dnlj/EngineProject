// Bench
#include <Bench/bench.hpp>
#include <Bench/Dist/Uniform.hpp>
#include <Bench/Dist/FixedStrings.hpp>

// STD
#include <cctype>

namespace {
	std::vector<std::string> truth(std::vector<std::string> vec) {
		for (auto& str : vec) {
			std::transform(str.begin(), str.end(), str.begin(), [](char c){ return std::tolower(c); });
		}
		return vec;
	}

	ENGINE_INLINE char toLowerSwitch(char c) noexcept {
		switch (c) {
			case 'A': return 'a';
			case 'B': return 'b';
			case 'C': return 'c';
			case 'D': return 'd';
			case 'E': return 'e';
			case 'F': return 'f';
			case 'G': return 'g';
			case 'H': return 'h';
			case 'I': return 'i';
			case 'J': return 'j';
			case 'K': return 'k';
			case 'L': return 'l';
			case 'M': return 'm';
			case 'N': return 'n';
			case 'O': return 'o';
			case 'P': return 'p';
			case 'Q': return 'q';
			case 'R': return 'r';
			case 'S': return 's';
			case 'T': return 't';
			case 'U': return 'u';
			case 'V': return 'v';
			case 'W': return 'w';
			case 'X': return 'x';
			case 'Y': return 'y';
			case 'Z': return 'z';
		}
		return c;
	}

	ENGINE_INLINE char toLowerTernary(char c) noexcept {
		return c >= 'A' && c <= 'Z' ? c+32 : c; // Also works: c|0x20
	}
}


BENCH(std_tolower) {
	static_assert(&tolower == &std::tolower);

	auto copy = dataset.internal();
	ctx.startSample();
	for (auto& str : copy) {
		for (auto& c : str) {
			c = std::tolower(c);
		}
	}
	ctx.stopSample();

	const auto ref = truth(dataset.internal());
	ctx.set("passed", ref == copy);
}

BENCH(switch_tolower) {
	static_assert(&tolower == &std::tolower);

	auto copy = dataset.internal();
	ctx.startSample();
	for (auto& str : copy) {
		for (auto& c : str) {
			c = toLowerSwitch(c);
		}
	}
	ctx.stopSample();

	const auto ref = truth(dataset.internal());
	ctx.set("passed", ref == copy);
}

BENCH(ternary_tolower) {
	static_assert(&tolower == &std::tolower);

	auto copy = dataset.internal();
	ctx.startSample();
	for (auto& str : copy) {
		for (auto& c : str) {
			c = toLowerTernary(c);
		}
	}
	ctx.stopSample();

	const auto ref = truth(dataset.internal());
	ctx.set("passed", ref == copy);
}



namespace {
	namespace D = Bench::Dist;
	// TODO: need to allow chars outside ['a', 'Z']
	using DataSet16 = D::FixedStrings<25'000, 16, 0>;
	using DataSet64 = D::FixedStrings<20'000, 64, 0>;
	using DataSet128 = D::FixedStrings<10'000, 128, 0>;
	using DataSet512 = D::FixedStrings<5'000, 512, 0>;
	// TODO: create a dataset from file names, cvarlist, etc
}

BENCH_GROUP("tolower", 2, 20);

BENCH_USE(std_tolower, DataSet16);
BENCH_USE(switch_tolower, DataSet16);
BENCH_USE(ternary_tolower, DataSet16);

BENCH_USE(std_tolower, DataSet64);
BENCH_USE(switch_tolower, DataSet64);
BENCH_USE(ternary_tolower, DataSet64);

BENCH_USE(std_tolower, DataSet128);
BENCH_USE(switch_tolower, DataSet128);
BENCH_USE(ternary_tolower, DataSet128);

BENCH_USE(std_tolower, DataSet512);
BENCH_USE(switch_tolower, DataSet512);
BENCH_USE(ternary_tolower, DataSet512);
