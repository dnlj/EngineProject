#pragma once

// Engine
#include <Engine/ECS/EntityManager.hpp>
#include <Engine/ECS/ComponentManager.hpp>
#include <Engine/ECS/SystemManager.hpp>

// TODO: Doc
// TODO: Enable once SystemManager is fixed
//namespace Engine::ECS {
//	class World : private EntityManager, private ComponentManager, private SystemManager {
//		public:
//			// EntityManager members
//			using EntityManager::createEntity;
//			using EntityManager::destroyEntity;
//			using EntityManager::isAlive;
//
//			// ComponentManager members
//			using ComponentManager::getComponentID;
//			using ComponentManager::getBitsetForComponents;
//			using ComponentManager::registerComponent;
//
//			// SystemManager members
//			using SystemManager::getSystemID;
//			using SystemManager::getSystem;
//			using SystemManager::getBitsetForSystems;
//			using SystemManager::registerSystem;
//	};
//}
