
namespace Engine::ECS {
	// TODO: make copy constructor on this and EntityFilter private
	template<bool IncludeDisabled, class C, class World>
	class SingleComponentFilter {
		private:
			using Cont = World::template ComponentContainer<C>;
			using ContIt = decltype(std::declval<Cont>().begin());
			using ContCIt = decltype(std::declval<Cont>().cbegin());
			World& world;

			ENGINE_INLINE static auto& getCont(World& w) { return w.template getComponentContainer<C>(); }
			ENGINE_INLINE static decltype(auto) getContBegin(World& w) { return getCont(w).begin(); }
			ENGINE_INLINE static decltype(auto) getContEnd(World& w) { return getCont(w).end(); }

			class Iter {
				private:
					friend class SingleComponentFilter;
					using I = int32;
					ContIt it;
					World& world;

					ENGINE_INLINE bool canUse(Entity ent) const {
						 // TODO: how would this happen assert fail in a valid
						 //       way? Before we were also checking that the
						 //       entity is alive but i think that would be a
						 //       bug?
						ENGINE_DEBUG_ASSERT(world.isAlive(ent));
						return IncludeDisabled || world.isEnabled(ent);
					}

					ENGINE_INLINE void stepNextValid() {
						const auto end = getContEnd(world);
						while (it != end && !canUse(it->first)) {
							++it;
						}
					}

					ENGINE_INLINE void stepPrevValid() {
						const auto begin = getContBegin(world);
						while (it != begin && !canUse(it->first)) {
							--it;
						}
					}

					static Iter begin(World& w) {
						Iter temp{getContBegin(w), w};
						temp.stepNextValid();
						return temp;
					}

					static Iter end(World& w) {
						Iter temp{getContEnd(w), w};
						return temp;
					}

					Iter(ContIt it, World& world) : it{it}, world{world} {
						// If we are including all components then we shouldn't be using this iterator.
						ENGINE_DEBUG_ASSERT(IncludeDisabled == false);
					}

				public:
					// Required for LegacyInputIterator
					using value_type = C;
					using reference = C&;
					using pointer = C*;
					using difference_type = int32;
					using iterator_category = std::input_iterator_tag;

					auto& operator++() {
						const auto end = getContEnd(world);
						if (it != end) { ++it; }
						stepNextValid();
						return *this;
					}

					auto& operator--() {
						const auto begin = getContBegin(world);
						if (it != begin) { --it; }
						stepPrevValid();
						return *this;
					}

					ENGINE_INLINE auto& operator*() {
						ENGINE_DEBUG_ASSERT(canUse(it->first), "Attempt to dereference invalid iterator.");
						return it->first;
					}

					ENGINE_INLINE decltype(auto) operator->() {
						return &**this;
					}

					ENGINE_INLINE bool operator==(const Iter& other) const noexcept { return it == other.it; }
					ENGINE_INLINE bool operator!=(const Iter& other) const noexcept { return !(*this == other); }
			};

		public:
			using Iterator = std::conditional_t<IncludeDisabled, ContIt, Iter>;

			// For STD compat
			using iterator = Iterator;
			using value_type = iterator::value_type;

			SingleComponentFilter(World& world) : world{world} {}

			SingleComponentFilter(const SingleComponentFilter&) = delete;
			SingleComponentFilter& operator=(const SingleComponentFilter&) = delete;

			SingleComponentFilter(SingleComponentFilter&&) = default;
			SingleComponentFilter& operator=(SingleComponentFilter&&) = default;

			ENGINE_INLINE Iterator begin() const {
				if constexpr (IncludeDisabled) {
					return getContBegin(world);
				} else {
					return Iter::begin(world);
				}
			}

			ENGINE_INLINE Iterator end() const {
				if constexpr (IncludeDisabled) {
					return getContEnd(world);
				} else {
					return Iter::end(world);
				}
			}

			ENGINE_INLINE Entity front() const {
				return *begin();
			}

			ENGINE_INLINE int32 size() const noexcept requires IncludeDisabled {
				return getCont(world).size();
			}

			ENGINE_INLINE bool empty() const { return begin() == end(); }
	};
}
