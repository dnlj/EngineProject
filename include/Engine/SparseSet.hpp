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
						typename Elem::Public>;
					Elem* curr;
					IteratorBase(Elem* init) : curr{init} {}

					// Verify our assumption
					static_assert(sizeof(Elem) == sizeof(AccessType));
					static_assert(offsetof(Elem, first) == offsetof(AccessType, first));
					static_assert(offsetof(Elem, second) == offsetof(AccessType, second));

				public:
					// TODO: iterator_traits
					
					auto& operator+=(Index i) { curr += i; return *this; }
					auto& operator-=(Index i) { curr -= i; return *this; }

					friend auto operator+(IteratorBase it, Index n) { return it += n; }
					friend auto operator-(IteratorBase it, Index n) { return it -= n; }

					friend auto operator+(Index n, IteratorBase it) { return it += n; }
					friend auto operator-(Index n, IteratorBase it) { return it -= n; }

					auto operator-(const IteratorBase& it) { return curr - it.curr; }

					auto& operator++() { ++curr; return *this; }
					auto& operator--() { --curr; return *this; }

					auto operator++(int) { return *this + 1; }
					auto operator--(int) { return *this - 1; }

					auto& operator*() const { return reinterpret_cast<AccessType&>(*curr); }
					auto* operator->() const { return &**this; }
					auto& operator[](Index n) const { return *(*this + n); }

					bool operator==(const IteratorBase& other) const { return curr == other.curr; }
					bool operator!=(const IteratorBase& other) const { return !(*this == other); }
					bool operator<(const IteratorBase& other) const { return curr < other.curr; }
					bool operator<=(const IteratorBase& other) const { return curr <= other.curr; }
					bool operator>=(const IteratorBase& other) const { return curr >= other.curr; }
					bool operator>(const IteratorBase& other) const { return curr > other.curr; }
			};

			using Iterator = IteratorBase<KeyVal>;
			using ConstIterator = IteratorBase<const KeyVal>;

		public:
			// TODO: EBO hash with Engine::BaseMember
			SparseSet(const Hash& hash = Hash()) : hash{hash} {}

			ENGINE_INLINE Val& operator[](Key key) {
				return get(key);
			}

			// We call this add because it is subtly different then the usage patterns of insert/emplace (see: std::unordered_map)
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

			[[nodiscard]] ENGINE_INLINE auto begin() {
				return Iterator{dense.data()};
			}

			[[nodiscard]] ENGINE_INLINE auto cbegin() const {
				return ConstIterator{dense.data()};
			}

			[[nodiscard]] ENGINE_INLINE auto begin() const {
				return cbegin();
			}

			[[nodiscard]] ENGINE_INLINE auto end() {
				return Iterator{dense.data() + dense.size()};
			}

			[[nodiscard]] ENGINE_INLINE auto cend() const {
				return ConstIterator{dense.data() + dense.size()};
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
