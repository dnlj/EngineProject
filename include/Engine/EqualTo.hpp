#pragma once

// STD
#include <functional>


namespace Engine {
	template<class T>
	struct EqualTo : std::equal_to<T> {
		using std::equal_to<T>::equal_to;
	};

	// TODO: where to put this?
	template<class C>
	struct EqualTo<std::basic_string<C>> {
		using is_transparent = void;

		[[nodiscard]]
		constexpr bool operator()(const std::basic_string<C>& left, const std::basic_string<C>& right) const noexcept {
			return left == right;
		}

		[[nodiscard]]
		constexpr bool operator()(std::basic_string_view<C> left, const std::basic_string<C>& right) const noexcept {
			return left == right;
		}

		[[nodiscard]]
		constexpr bool operator()(const C* left, const std::basic_string<C>& right) const noexcept {
			return std::basic_string_view<C>(left) == right;
		}
	};
};
