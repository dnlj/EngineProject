#pragma once

// STD
#include <string>
#include <array>
#include <random>

// PCG
#include <pcg_random.hpp>


namespace Bench::Dist {
	template<int64 N, int64 S, int64 seed = 0>
	class FixedStrings {
		private:
			std::vector<std::string> storage;

		public:
			FixedStrings() {
				pcg32_k16384 rng = Bench::seeds[seed];
				storage.resize(N);

				for (auto& str : storage) {
					str.resize(S);
					std::ranges::generate(str, [&]{ return ' ' + rng('~' - ' '); });
				}
			}

			decltype(auto) begin() const { return storage.begin(); }
			decltype(auto) end() const { return storage.end(); }
			constexpr static int64 size() noexcept { return N; }
	};
}
