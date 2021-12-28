#pragma once

// STD
#include <tuple>
#include <utility>

// Engine
#include <Engine/Meta/ForEach.hpp>


// TODO: everything in this file should probably be moved into Engine::*
namespace Bench::Dist {

	/**
	 * Forwards the type unless it is an rvalue reference, in which case resolves to a non-reference type.
	 */
	template<class T>
	struct RemoveRValueRef {
		using type = T;
	};
	
	template<class T>
	struct RemoveRValueRef<T&&> {
		using type = T;
	};

	template<class T>
	using RemoveRValueRef_t = typename RemoveRValueRef<T>::type;


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

	template<class... Iters>
	class ZipIterator {
		private:
			std::tuple<Iters...> iters;

		public:
			ZipIterator(Iters... its) : iters{its...} {}

			ZipIterator(const std::tuple<Iters...>& its) : iters{its} {}
			ZipIterator(std::tuple<Iters...>&& its) : iters{std::move(its)} {}

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

			ENGINE_INLINE friend bool operator==(const ZipIterator& lhs, const ZipIterator& rhs) noexcept {
				return lhs.iters == rhs.iters;
			}

			ENGINE_INLINE friend bool operator<=>(const ZipIterator& lhs, const ZipIterator& rhs) noexcept {
				return lhs.iters <=> rhs.iters;
			}
	};

	template<class... Ranges>
	class Zip {
		private:
			static_assert((!std::is_rvalue_reference_v<Ranges> && ...), "Cannot store rvalue reference");
			std::tuple<Ranges...> ranges; // RemoveRValueRef_t no longer needed here because of ctad

		public:
			Zip() {}

			template<class... Rs>
			Zip(Rs&&... rs) : ranges{std::forward<Rs>(rs)...} {}

			// TODO: constructor from tuple with deduction guidelines?

			constexpr auto size() {
				return std::apply([](auto&&... args){
					return std::min({std::ranges::size(args)...}); },
				ranges);
			}

			auto begin() { return ZipIterator(forEach(ranges, [](auto& r){ return std::ranges::begin(r); })); }
			auto begin() const { return ZipIterator(forEach(ranges, [](auto& r){ return std::ranges::begin(r); })); }
			auto cbegin() const { return ZipIterator(forEach(ranges, [](auto& r){ return std::ranges::cbegin(r); })); }
			
			auto end() { return ZipIterator(forEach(ranges, [](auto& r){ return std::ranges::end(r); })); }
			auto end() const { return ZipIterator(forEach(ranges, [](auto& r){ return std::ranges::end(r); })); }
			auto cend() const { return ZipIterator(forEach(ranges, [](auto& r){ return std::ranges::cend(r); })); }

			friend bool operator==(const Zip& lhs, const Zip& rhs) noexcept {
				return lhs.begin() == rhs.begin() && lhs.end() == rhs.end();
			}

			friend bool operator<=>(const Zip& lhs, const Zip& rhs) noexcept {
				return std::tuple{lhs.begin(), lhs.end()} <=> std::tuple{rhs.begin(), rhs.end()};
			}
	};

	// Needed to allow forwarding and stripping of rvalue reference. Resolves as: T& -> T&, T&& -> T, T -> T
	template<class... Ts> Zip(Ts&&...) -> Zip<Ts...>;
}
