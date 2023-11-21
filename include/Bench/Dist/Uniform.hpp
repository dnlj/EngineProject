#pragma once

// STD
#include <ranges>
#include <random>

// PCG
#include <pcg_random.hpp>


namespace Bench::Dist {
	template<class T, int64 N, int64 seed = 0>
	struct Uniform {
		using ValueType = T;
		std::array<T, N> storage;
		using D = std::conditional_t<std::is_floating_point_v<T>, std::uniform_real_distribution<T>, std::uniform_int_distribution<T>>;

		Uniform() {
			pcg32_k16384 rng = Bench::seeds[seed];

			// TODO: Is there a reason this was only positive numbers?
			//       Does this break anything?
			//D dist(0, std::numeric_limits<T>::max());

			constexpr auto lower = []() -> T {
				if constexpr (std::is_floating_point_v<T>) {
					//return std::nextafter(std::numeric_limits<T>::lowest(), std::numeric_limits<T>::max());
					//return -std::nextafter(std::numeric_limits<T>::max(), T{});
					return 0; // TODO: allow negative numbers, idk why the above doesn't work?
				} else {
					return std::numeric_limits<T>::lowest();
				}
			};

			D dist(lower(), std::numeric_limits<T>::max());
			for (int64 i = 0; i < N; ++i) {
				storage[i] = dist(rng);
			}
		}

		decltype(auto) begin() const { return storage.begin(); }
		decltype(auto) end() const { return storage.end(); }

		constexpr static int64 size() noexcept { return N; }
	};
}
