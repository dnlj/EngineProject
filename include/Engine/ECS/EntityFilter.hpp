#pragma once

// STD
#include <vector>

// Engine
#include <Engine/ECS/Common.hpp>

// TODO: Doc
namespace Engine::ECS {
	class EntityFilter {
		private:
			std::vector<Entity> entities;

		public:
			using ConstIterator = decltype(entities)::const_iterator;

			EntityFilter(ComponentBitset componentBits);

			const ComponentBitset componentBits;

			void addEntity(Entity ent);

			ConstIterator begin() const;
			ConstIterator end() const;

			Entity operator[](std::size_t index) const;
	};
}
