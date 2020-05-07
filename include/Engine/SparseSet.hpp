#pragma once

// STD
#include <vector>
#include <utility>

// Engine
#include <Engine/Engine.hpp>


namespace Engine {
	template<class Key, class Val>
	class SparseSet {
		private:
			static constexpr Key invalid = static_cast<Key>(0 - 1);
			using KeyVal = std::pair<Key, Val>;

			std::vector<Key> sparse;
			std::vector<KeyVal> dense;

		public:
			ENGINE_INLINE Val& operator[](Key key) {
				return get(key);
			}

			template<class... Args>
			Val& add(Key key, Args&&... args) {
				if (sparse.size() <= key) { sparse.resize(key + 1, invalid); }
				ENGINE_DEBUG_ASSERT(sparse[key] == invalid, "Adding duplicate key to set.");
				sparse[key] = static_cast<Key>(dense.size());
				return dense.emplace_back(
					std::piecewise_construct,
					std::forward_as_tuple(key),
					std::forward_as_tuple(std::forward<Args>(args)...)
				).second;
			}

			ENGINE_INLINE void remove(Key key) {
				using std::swap;
				sparse[dense.back().key] = sparse[key];
				sparse[key] = invalid;
				swap(dense.back(), get(key));
				dense.pop_back();
			}

			ENGINE_INLINE bool has(Key key) const {
				return key < sparse.size() && sparse[key] != invalid;
			}

			ENGINE_INLINE Val& get(Key key) {
				return dense[sparse[key]].second;
			}

			ENGINE_INLINE Val& get(Key key) const {
				return dense[sparse[key]].second;
			}

			ENGINE_INLINE Key size() const {
				return dense.size();
			}

			// TODO: sort

			// TODO: shrink
	};
}
