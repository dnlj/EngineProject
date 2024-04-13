#pragma once

// STD
#include <vector>
#include <type_traits>
#include <utility>
#include <tuple>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/IndexHash.hpp>


namespace Engine {
	// TODO (C++23): Fix type punning with start_lifetime_as+byte storage where appropriate. Looks like just the KeyVal stuff.
	template<class Key, class Value, class Hash = IndexHash<Key>>
	class SparseSet {
		private:
			using Index = int32;
			static constexpr bool IsVoid = std::is_same_v<Value, void>;
			static constexpr Index invalid = static_cast<Index>(-1);
			using Val = std::conditional_t<IsVoid, Key, Value>;

			union KeyVal_void {
				KeyVal_void(const Key& key) : first{key} {}
				Key first;
				Val second;

				union Public {
					Public(const KeyVal_void& kv) : first{kv.first} {}
					const Key first;
					const Val second;
				};
			};

			class KeyVal_value {
				public:
					Key first;
					Val second;

					struct Public {
						const Key first;
						Val second;
					};

					template<class... Args1, class... Args2>
					KeyVal_value(std::piecewise_construct_t,
						const std::tuple<Args1...>& args1,
						const std::tuple<Args2...>& args2)
						: KeyVal_value{std::piecewise_construct, args1, args2, std::index_sequence_for<Args1...>{}, std::index_sequence_for<Args2...>{}} {
					};

				private:
					template<class... Args1, class... Args2, size_t... I1, size_t... I2>
					KeyVal_value(std::piecewise_construct_t,
						const std::tuple<Args1...>& args1,
						const std::tuple<Args2...>& args2,
						std::index_sequence<I1...>,
						std::index_sequence<I2...>)
						: first(std::forward<Args1>(std::get<I1>(args1))...)
						, second(std::forward<Args2>(std::get<I2>(args2))...) {
					};
			};
			
			using KeyVal = std::conditional_t<IsVoid, KeyVal_void, KeyVal_value>;

			Hash hash;
			std::vector<Index> sparse;
			std::vector<KeyVal> dense;

			template<class Elem>
			class IteratorBase {
				private:
					friend class SparseSet;
					// TODO: create a copy_cv<A, B> type trait
					using AccessType = std::conditional_t<
						std::is_const_v<Elem>,
						const typename Elem::Public,
						typename Elem::Public
					>;

					Elem* curr;
					ENGINE_DEBUG_ONLY(Elem* end = nullptr);

					IteratorBase(Elem* init) : curr{init} {}

					#if ENGINE_DEBUG
						IteratorBase(Elem* init, Elem* end) : curr{init}, end{end} {};
						ENGINE_INLINE void check() const { ENGINE_DEBUG_ASSERT(curr < end, "Attempting to use invalid SparseSet iterator."); };
					#endif

					// Verify our assumption
					static_assert(sizeof(Elem) == sizeof(AccessType));
					static_assert(offsetof(Elem, first) == offsetof(AccessType, first));
					static_assert(offsetof(Elem, second) == offsetof(AccessType, second));

				public:
					// TODO: iterator_traits
					// For STD compat
					using value_type = AccessType;
					
					ENGINE_INLINE auto& operator+=(Index i) { curr += i; return *this; }
					ENGINE_INLINE auto& operator-=(Index i) { curr -= i; return *this; }

					ENGINE_INLINE friend auto operator+(IteratorBase it, Index n) { return it += n; }
					ENGINE_INLINE friend auto operator-(IteratorBase it, Index n) { return it -= n; }

					ENGINE_INLINE friend auto operator+(Index n, IteratorBase it) { return it += n; }
					ENGINE_INLINE friend auto operator-(Index n, IteratorBase it) { return it -= n; }

					ENGINE_INLINE auto operator-(const IteratorBase& it) { return curr - it.curr; }

					ENGINE_INLINE auto& operator++() { ++curr; return *this; }
					ENGINE_INLINE auto& operator--() { --curr; return *this; }

					ENGINE_INLINE auto operator++(int) { return *this + 1; }
					ENGINE_INLINE auto operator--(int) { return *this - 1; }

					ENGINE_INLINE_REL auto& operator*() const { ENGINE_DEBUG_ONLY(check()); return reinterpret_cast<AccessType&>(*curr); }
					ENGINE_INLINE_REL auto& operator[](Index n) const { return *(*this + n); }

					// Technically we need to specialize std::pointer_traits<Ptr>::to_address to
					// return curr directly instead of reusing operator*() because to_address
					// must return the address without forming a reference and will default to
					// operator->() without a pointer_traits specialization.
					//
					// In practice that isn't an issue right now and we want to explicitly
					// disallow that to help catch bugs in debug mode. We may need to add it
					// later to be compliant, but for now we just avoid calling std::to_address
					// on out-of-bounds iterators.
					ENGINE_INLINE_REL auto* operator->() const { return &**this; }

					ENGINE_INLINE bool operator==(const IteratorBase& other) const { return curr == other.curr; }
					ENGINE_INLINE bool operator!=(const IteratorBase& other) const { return !(*this == other); }
					ENGINE_INLINE bool operator<(const IteratorBase& other) const { return curr < other.curr; }
					ENGINE_INLINE bool operator<=(const IteratorBase& other) const { return curr <= other.curr; }
					ENGINE_INLINE bool operator>=(const IteratorBase& other) const { return curr >= other.curr; }
					ENGINE_INLINE bool operator>(const IteratorBase& other) const { return curr > other.curr; }
			};

		public:
			using Iterator = IteratorBase<KeyVal>;
			using ConstIterator = IteratorBase<const KeyVal>;

			// TODO: EBO hash with Engine::BaseMember
			SparseSet(const Hash& hash = Hash()) : hash{hash} {}

			ENGINE_INLINE Val& operator[](Key key) {
				return get(key);
			}

			// We call this add because it is subtly different then the usage patterns of insert/emplace (see: std::unordered_map)
			// TODO (NEtwNxsp): I assume the above is referencing that unordered_map takes a pair? Probably rename to emplace anyways (std now has try_emplace with this same interface)
			template<class... Args>
			auto& add(const Key& key, Args&&... args) {
				const auto i = hash(key);
				if (sparse.size() <= i) { sparse.resize(i + 1, invalid); }
				ENGINE_DEBUG_ASSERT(sparse[i] == invalid, "Adding duplicate key to set.");
				sparse[i] = static_cast<Index>(dense.size());

// TODO: if we just store key for dense.back() we dont need to use a tuple for value array. But this would make sorting more difficult. If we go with sorting by sparse array this just storing last might be a good idea.
				if constexpr (IsVoid) {
					return dense.emplace_back(key);
				} else {
					return dense.emplace_back(
						std::piecewise_construct,
						std::forward_as_tuple(key),
						std::forward_as_tuple(std::forward<Args>(args)...)
					).second;
				}
			}

			ENGINE_INLINE void clear() {
				sparse.clear();
				dense.clear();
			}

			ENGINE_INLINE void erase(const Key& key) {
				const auto i = hash(key);
				sparse[hash(dense.back().first)] = sparse[i];
				dense[sparse[i]] = std::move(dense.back());
				sparse[i] = invalid;
				dense.pop_back();
			}

			[[nodiscard]] ENGINE_INLINE bool contains(const Key& key) const {
				const auto i = hash(key);
				return i < sparse.size() && sparse[i] != invalid;
			}

			[[nodiscard]] Val& get(const Key& key) {
				return dense[sparse[hash(key)]].second;
			}

			[[nodiscard]] ENGINE_INLINE const Val& get(const Key& key) const {
				return dense[sparse[hash(key)]].second;
			}

			[[nodiscard]] ENGINE_INLINE Index size() const {
				return static_cast<Index>(dense.size());
			}

			// TODO: split into erase and remove once we fix iterators (probably after we have other sorting functions implemented)
			/**
			 * TODO: desc
			 * Guaranteed to call the predicate on each element exactly once.
			 * @param pred The predicate check against.
			 */
			template<class Pred>
			void eraseRemove(Pred&& pred) {
				// TODO: use our own iterators once we fix the move issues
				const auto b = dense.begin();
				const auto e = dense.end();

				auto curr = b;
				auto stop = e;

				while (curr < stop) {
					if (pred(*curr)) {
						--stop;
						auto& spc = sparse[hash(curr->first)];
						sparse[hash(stop->first)] = spc;
						spc = invalid;
						*curr = std::move(*stop);
					} else {
						++curr;
					}
				}

				dense.erase(stop, e);
			}

			// TODO: sort

			// TODO: shrink

			// TODO: empty

			[[nodiscard]] ENGINE_INLINE_REL auto begin() {
				return Iterator{dense.data() ENGINE_DEBUG_ONLY(ENGINE_COMMA std::to_address(dense.end())) };
			}

			[[nodiscard]] ENGINE_INLINE auto cbegin() const {
				return ConstIterator{dense.data() ENGINE_DEBUG_ONLY(ENGINE_COMMA std::to_address(dense.end())) };
			}

			[[nodiscard]] ENGINE_INLINE auto begin() const {
				return cbegin();
			}

			[[nodiscard]] ENGINE_INLINE auto end() {
				return Iterator{dense.data() + dense.size() ENGINE_DEBUG_ONLY(ENGINE_COMMA std::to_address(dense.end())) };
			}

			[[nodiscard]] ENGINE_INLINE auto cend() const {
				return ConstIterator{dense.data() + dense.size() ENGINE_DEBUG_ONLY(ENGINE_COMMA std::to_address(dense.end())) };
			}

			[[nodiscard]] ENGINE_INLINE auto end() const {
				return cend();
			}

			friend void swap(SparseSet& a, SparseSet& b) {
				using std::swap;
				swap(a.hash, b.hash);
				swap(a.sparse, b.sparse);
				swap(a.dense, b.dense);
			}
	};
}
