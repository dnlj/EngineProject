#pragma once

// STD
#include <vector>
#include <iterator>

// Engine
#include <Engine/ECS/Common.hpp>
#include <Engine/ECS/EntityManager.hpp>


// TODO: Doc
namespace Engine::ECS {
	class EntityFilter {
		private:
			std::vector<Entity> entities;

		private:
			template<class T>
			class Iterator {
				private:
					typename std::vector<std::remove_const_t<T>>::const_iterator it;

				public:
					using difference_type = std::ptrdiff_t;
					using value_type = T;
					using pointer = value_type*;
					using reference = value_type&;
					using iterator_category = std::bidirectional_iterator_tag;
					
					Iterator() = default;

					// TODO: move
					Iterator(decltype(it) it) : it(it) {};

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
						return const_cast<const Iterator*>(this)->();
					}

					// TODO: Move
					T* operator->() const {
						return &*it;
					}

					// TODO: Move
					Iterator& operator++() {
						++it;
						return *this;
					};

					// TODO: Move
					Iterator& operator--() {
						--it;
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

			// TODO: Redfine this in terms of size
			bool empty() const;

			// TODO: Will need to make this track the number of active entities
			std::size_t size() const;

			ConstIterator begin() const;
			ConstIterator end() const;

			ConstIterator cbegin() const;
			ConstIterator cend() const;

		private:
			EntityManager& entityManager;
	};
}
