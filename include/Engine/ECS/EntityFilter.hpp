#pragma once

// STD
#include <vector>

// Engine
#include <Engine/ECS/Entity.hpp>


namespace Engine::ECS {
	class EntityFilter {
		private:
			std::vector<Entity> entities;

		public:
			using ConstIterator = decltype(entities)::const_iterator;

			ConstIterator begin() const;
			ConstIterator end() const;

			Entity operator[](std::size_t index) const;
	};
}
