#pragma once

// STD
#include <ranges>
#include <random>

// PCG
#include <pcg_random.hpp>


namespace Bench::Dist {
	template<class T, size_t N>
	struct Uniform {
		std::array<T, N> storage;
		using D = std::conditional_t<std::is_floating_point_v<T>, std::uniform_real_distribution<T>, std::uniform_int_distribution<T>>;

		Uniform() {
			pcg32_k16384 rng = pcg_extras::seed_seq_from<std::random_device>();
			D dist(0, std::numeric_limits<T>::max());
			for (size_t i = 0; i < N; ++i) {
				storage[i] = dist(rng);
			}
		}

		decltype(auto) begin() const { return storage.begin(); }
		decltype(auto) end() const { return storage.end(); }
	};
}
