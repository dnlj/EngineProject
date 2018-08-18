#pragma once

// STD
#include <vector>
#include <iterator>

// Engine
#include <Engine/ECS/Common.hpp>
#include <Engine/ECS/EntityManager.hpp>
#include <Engine/Engine.hpp>


// TODO: Doc
namespace Engine::ECS {
	class EntityFilter {
		private:
			std::vector<Entity> entities;

		private:
			template<class T>
			class Iterator {
				private:
					using ItType = typename std::vector<std::remove_const_t<T>>::const_iterator;
					ItType it;
					const EntityFilter& filter;

				public:
					using difference_type = std::ptrdiff_t;
					using value_type = T;
					using pointer = value_type*;
					using reference = value_type&;
					using iterator_category = std::bidirectional_iterator_tag;
					
					Iterator() = default;

					// TODO: move
					Iterator(const EntityFilter& filter, ItType it)
						: filter(filter)
						, it(it) {
					};

					// TODO: Move
					T& operator*() {
						return **const_cast<const Iterator*>(this);
					};

					// TODO: Move
					T& operator*() const {
						return *it;
					}

					// TODO: Move
					T* operator->() {
						return const_cast<const Iterator*>(this)->(); // TODO: is this correct?
					}

					// TODO: Move
					T* operator->() const {
						return &*it;
					}

					// TODO: Move
					Iterator& operator++() {
						auto end = filter.end();

						#if defined(DEBUG)
							if (*this == end) {
								ENGINE_ERROR("Attempting to increment an end iterator");
							}
						#endif

						while ((++it, *this) != end && !filter.entityManager.isEnabled(*it)) {
						}

						return *this;
					};

					// TODO: Move
					Iterator& operator--() {
						auto begin = filter.begin();

						#if defined(DEBUG)
							if (*this == begin) {
								ENGINE_ERROR("Attempting to decrement an begin iterator");
							}
						#endif

						while ((--it, *this) != begin && !filter.entityManager.isEnabled(*it)) {
						}

						return *this;
					}

					// TODO: Move
					Iterator operator++(int) {
						auto temp = *this;
						++*this;
						return temp;
					};

					// TODO: Move
					Iterator operator--(int) {
						auto temp = *this;
						--*this;
						return temp;
					}

					// TODO: Move
					friend bool operator==(const Iterator& first, const Iterator& second) {
						return first.it == second.it;
					};

					// TODO: Move
					friend bool operator!=(const Iterator& first, const Iterator& second) {
						return !(first == second);
					};
			};

		public:
			using ConstIterator = Iterator<const Entity>;

			// TODO: Make private. Move to constructor
			ComponentBitset componentsBits;

			EntityFilter(EntityManager& entityManager);

			void add(Entity ent, const ComponentBitset& cbits);
			void remove(Entity ent);

			std::size_t size() const;
			bool empty() const;

			ConstIterator begin() const;
			ConstIterator end() const;

			ConstIterator cbegin() const;
			ConstIterator cend() const;

		private:
			EntityManager& entityManager;
	};
}
