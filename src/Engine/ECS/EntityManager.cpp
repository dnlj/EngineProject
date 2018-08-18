// Engine
#include <Engine/ECS/EntityManager.hpp>

#if defined(DEBUG)
	#include <Engine/Engine.hpp>
#endif

namespace Engine::ECS {
	Entity EntityManager::createEntity(bool forceNew) {
		Entity ent;

		if (!forceNew && !deadEntities.empty()) {
			ent = deadEntities.back();
			deadEntities.pop_back();
		} else {
			ent = Entity{static_cast<decltype(Entity::id)>(aliveEntities.size()), 0};
			aliveEntities.resize(ent.id + 1);
			enabledEntities.resize(ent.id + 1);
		}

		++ent.gen;
		aliveEntities[ent.id] = ent.gen;
		enabledEntities[ent.id] = true;

		return ent;
	}

	void EntityManager::destroyEntity(Entity ent) {
		#if defined(DEBUG)
			if (!isAlive(ent)) {
				ENGINE_ERROR("Attempting to destroy an already dead entity \"" << ent << "\"");
			} else if (aliveEntities[ent.id] != ent.gen) {
				ENGINE_ERROR(
					"Attempting to destroy an old generation entity. Current generation is "
					<< aliveEntities[ent.id]
					<< " attempted to delete "
					<< ent.gen
				);
			}
		#endif

		deadEntities.emplace_back(std::move(ent));
		aliveEntities[ent.id] = 0;
	}

	auto EntityManager::getEntities() const -> const EntityContainer& {
		return aliveEntities;
	}

	bool EntityManager::isAlive(Entity ent) const {
		return aliveEntities[ent.id];
	}

	void EntityManager::setEnabled(Entity ent, bool enabled) {
		enabledEntities[ent.id] = enabled;
	}

	bool EntityManager::isEnabled(Entity ent) const {
		return enabledEntities[ent.id];
	}
}
