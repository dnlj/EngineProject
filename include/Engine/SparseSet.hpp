#pragma once

// STD
#include <vector>
#include <utility>
#include <type_traits>
#include <concepts>

// Engine
#include <Engine/Engine.hpp>


namespace Engine {
	// TODO: move
	// TODO: name? dense, unique, sequential, key
	template<class T>
	struct IndexHash {
		int32 operator()(const T& v) const {
			static_assert(false, "IndexHash is not specialized for this type");
		}
	};

	template<std::integral T>
	struct IndexHash<T> {
		T operator()(const T& v) const {
			return v;
		}
	};

	template<>
	struct IndexHash<ECS::Entity> {
		int32 operator()(const ECS::Entity& v) const {
			return v.id;
		}
	};

	template<class Key, class Value, class Hash = IndexHash<Key>>
	class SparseSet {
		private:
			using Index = int32;
			static constexpr bool IsVoid = std::is_same_v<Value, void>;
			static constexpr Index invalid = static_cast<Index>(-1);

			union KeyVal_void {
				KeyVal_void(const Key& key) : first{key} {}
				// TODO: want these to be const from iterators. Should not be modified.
				Key first;
				Key second;
			};
			using Val = std::conditional_t<IsVoid, Key, Value>;
			using KeyVal = std::conditional_t<IsVoid, KeyVal_void, std::pair<Key, Val>>;
			Hash hash;
			std::vector<Index> sparse;
			std::vector<KeyVal> dense;

		public:
			SparseSet(const Hash& hash = Hash()) : hash{hash} {}

			ENGINE_INLINE Val& operator[](Key key) {
				return get(key);
			}

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

			ENGINE_INLINE void remove(const Key& key) {
				const auto i = hash(key);
				sparse[dense.back().first] = sparse[i];
				dense[sparse[i]] = std::move(dense.back());
				sparse[i] = invalid;
				dense.pop_back();
			}

			ENGINE_INLINE bool has(const Key& key) const {
				const auto i = hash(key);
				return i < sparse.size() && sparse[i] != invalid;
			}

			ENGINE_INLINE Val& get(const Key& key) {
				return dense[sparse[hash(key)]].second;
			}

			ENGINE_INLINE Val& get(const Key& key) const {
				return dense[sparse[hash(key)]].second;
			}

			ENGINE_INLINE Index size() const {
				return static_cast<Index>(dense.size());
			}

			// TODO: sort

			// TODO: shrink

			// TODO: empty

			ENGINE_INLINE auto begin() {
				// TODO: we need `it.first` to be const. In case of IsVoid both should be const.
				return dense.begin();
			}

			ENGINE_INLINE auto begin() const {
				return cbegin();
			}

			ENGINE_INLINE auto cbegin() {
				return dense.cbegin();
			}

			ENGINE_INLINE auto end() {
				return dense.end();
			}

			ENGINE_INLINE auto end() const {
				return cend;
			}

			ENGINE_INLINE auto cend() {
				return dense.cend();
			}

			friend void swap(SparseSet& a, SparseSet& b) {
				using std::swap;
				swap(a.hash, b.hash);
				swap(a.sparse, b.sparse);
				swap(a.dense, b.dense);
			}
	};
}
