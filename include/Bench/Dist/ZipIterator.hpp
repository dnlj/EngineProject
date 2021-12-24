#pragma once

// STD
#include <tuple>
#include <utility>

// Engine
#include <Engine/Meta/ForEach.hpp>


// TODO: everything in this file should probably be moved into Engine::*
namespace Bench::Dist {
	// TODO: probably move into Engine::Meta or similar
	template<class T, class... Us>
	struct IsAnyOf : std::bool_constant<(std::same_as<T, Us> || ...)> {};

	template<class T, class... Us>
	constexpr inline bool IsAnyOf_v = IsAnyOf<T, Us...>::value;

	class Nothing {};

	/**
	 * Calls a callable with the given arguments and returns the result.
	 * If the result is `void`, return a Nothing object instead.
	 */
	template<class Func, class... Args>
	// TODO: better name
	ENGINE_INLINE decltype(auto) call(Func&& func, Args&&... args) {
		if constexpr (std::same_as<void, decltype(std::forward<Func>(func)(std::forward<Args>(args)...))>) {
			std::forward<Func>(func)(std::forward<Args>(args)...);
			return Nothing{};
		} else {
			return std::forward<Func>(func)(std::forward<Args>(args)...);
		}
	}

	template<class Tuple, class Func, auto... Is>
	ENGINE_INLINE decltype(auto) forEach(Tuple&& tuple, Func&& func, std::index_sequence<Is...>) {
		using Ret = std::tuple<decltype(call(std::forward<Func>(func), std::get<Is>(std::forward<Tuple>(tuple))))...>;
		return Ret(call(std::forward<Func>(func), std::get<Is>(std::forward<Tuple>(tuple)))...);
	}

	template<class Tuple, class Func>
	ENGINE_INLINE decltype(auto) forEach(Tuple&& tuple, Func&& func) {
		return forEach(std::forward<Tuple>(tuple), std::forward<Func>(func), std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tuple>>>{});
	}

	// TODO: Zipper<Cont1, Cont2, ...> class (range) for begin/end
	// TODO: zip(x,y,z) function that returns a Zipper<X,Y,Z? 

	template<class... Iters>
	class ZipIterator {
		private:
			std::tuple<Iters...> iters;

		public:
			ZipIterator(Iters... its) : iters{its...} {}

			decltype(auto) operator*() {
				return forEach(iters, [](auto&& arg) ENGINE_INLINE { return *arg; });
			}

			auto operator++(int) {
				const auto old = *this;
				++*this;
				return old;
			}

			auto& operator++() {
				forEach(iters, [](auto&& arg) ENGINE_INLINE { ++arg; });
				return *this;
			}
			
			auto& operator+=(int64 n) {
				forEach(iters, [n](auto&& arg) ENGINE_INLINE { arg += n; });
				return *this;
			}

			auto operator--(int) {
				const auto old = *this;
				++*this;
				return old;
			}

			auto& operator--() {
				forEach(iters, [](auto&& arg) ENGINE_INLINE { ++arg; });
				return *this;
			}

			auto& operator-=(int64 n) {
				forEach(iters, [n](auto&& arg) ENGINE_INLINE { arg -= n; });
				return *this;
			}

			ENGINE_INLINE bool operator<=>(const ZipIterator& other) const noexcept {
				return iters <=> other.iters;
			}
	};
}
