#pragma once

// STD
#include <tuple>
#include <utility>

// Engine
#include <Engine/tuple.hpp>


namespace Engine {
	/**
	 * Wraps a set of iterators; performs operations on every iterator.
	 */
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
			std::tuple<Ranges...> ranges; // RemoveRValueRef_t no longer needed here because of ctad and static_assert

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
