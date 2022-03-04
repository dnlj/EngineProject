#pragma once


namespace Engine::Algorithm {
	template<class T>
	constexpr void swap(T& a, T& b) {
		T temp = std::move(a);
		a = std::move(b);
		b = std::move(temp);
	}

	template<class T, std::size_t N>
	constexpr auto pivot(std::array<T, N>& arr, int b, int e) {
		auto m = (b + e) / 2;

		if (arr[m] < arr[b]) {
			swap(arr[m], arr[b]);
		}

		if (arr[e] < arr[b]) {
			swap(arr[e], arr[b]);
		}

		if (arr[m] < arr[e]) {
			swap(arr[m], arr[e]);
		}

		return arr[m];
	}

	template<class T, std::size_t N>
	constexpr int partition(std::array<T, N>& arr, int b, int e) {
		auto p = pivot(arr, b, e);
		--b;
		++e;

		while (true) {
			do {
				++b;
			} while (arr[b] < p);
		
			do {
				--e;
			} while (arr[e] > p);
		
			if (b >= e) {
				return e;
			}
		
			swap(arr[b], arr[e]);
		}
	}

	template<class T, std::size_t N>
	constexpr void quicksort(std::array<T, N>& arr, int b, int e) {
		if (b < e) {
			auto p = partition(arr, b, e);
			quicksort(arr, b, p);
			quicksort(arr, p + 1, e);
		}
	}

	template<class T, std::size_t N>
	constexpr auto sort(std::array<T, N> arr) {
		if constexpr (N > 0) {
			quicksort(arr, 0, N - 1);
		}

		return arr;
	}
}
