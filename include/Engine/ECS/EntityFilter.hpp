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
					Iterator(const EntityFilter& filter, ItType it);

					T& operator*();
					T& operator*() const;

					T* operator->();
					T* operator->() const;

					Iterator& operator++();
					Iterator& operator--();

					Iterator operator++(int);
					Iterator operator--(int);

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

			EntityFilter(const EntityManager& entityManager);

			void add(Entity ent, const ComponentBitset& cbits);
			void remove(Entity ent);

			std::size_t size() const;
			bool empty() const;

			ConstIterator begin() const;
			ConstIterator end() const;

			ConstIterator cbegin() const;
			ConstIterator cend() const;

		private:
			const EntityManager& entityManager;
	};
}
