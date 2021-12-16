#pragma once

// STD
#include <ranges>
#include <random> // TODO: rm - use pcg


namespace Bench::Dist {
	template<class T, size_t N>
	struct Uniform {
		std::vector<T> storage;

		Uniform() {
			for (T i = 0; i < N; ++i) {
				storage.push_back(i);
			}

			// TODO: PCG - fixed seed
			std::random_device rd;
			std::mt19937 gen{rd()};
			std::ranges::shuffle(storage, gen);
		}

		decltype(auto) begin() const { return storage.begin(); }
		decltype(auto) end() const { return storage.end(); }
	};
}
