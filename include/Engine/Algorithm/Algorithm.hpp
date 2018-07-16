#pragma once

// STD
#include <array>

// TODO: Doc
namespace Engine::Algorithm {
	// TODO: C++20 has constexpr std::swap
	template<class T>
	constexpr void swap(T& a, T& b);

	template<class T, std::size_t N>
	constexpr auto pivot(std::array<T, N>& arr, int b, int e);

	template<class T, std::size_t N>
	constexpr int partition(std::array<T, N>& arr, int b, int e);

	// NOTE: Doesnt work with duplicate elements
	template<class T, std::size_t N>
	constexpr void quicksort(std::array<T, N>& arr, int b, int e);

	// TODO: C++20 has constexpr std::sort
	template<class T, std::size_t N>
	constexpr auto sort(std::array<T, N> arr);
}

#include <Engine/Algorithm/Algorithm.ipp>
