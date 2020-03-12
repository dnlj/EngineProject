#pragma once

// Meta
#include <Meta/TypeSet/TypeSet.hpp>

// Engine
#include <Engine/ECS/World.hpp>
#include <Engine/EngineInstance.hpp>

// Game
#include <Game/CharacterMovementSystem.hpp>
#include <Game/PhysicsSystem.hpp>
#include <Game/CharacterSpellSystem.hpp>
#include <Game/CameraTrackingSystem.hpp>
#include <Game/MapSystem.hpp>
#include <Game/MapRenderSystem.hpp>
#include <Game/SpriteSystem.hpp>

#include <Game/PhysicsComponent.hpp>
#include <Game/SpriteComponent.hpp>
#include <Game/CharacterMovementComponent.hpp>
#include <Game/CharacterSpellComponent.hpp>
#include <Game/InputComponent.hpp>


namespace Game {
	using SystemsSet = Meta::TypeSet::TypeSet<
		CharacterMovementSystem,
		PhysicsSystem,
		CharacterSpellSystem,
		CameraTrackingSystem,
		MapSystem,
		MapRenderSystem,
		SpriteSystem
	>;
	
	using ComponentsSet = Meta::TypeSet::TypeSet<
		PhysicsComponent,
		SpriteComponent,
		CharacterMovementComponent,
		CharacterSpellComponent,
		InputComponent
	>;

	class World : public Engine::ECS::World<SystemsSet, ComponentsSet> {
		public:
			World(float tickInterval, Engine::EngineInstance& engine)
				: Engine::ECS::World<SystemsSet, ComponentsSet>(tickInterval, std::tie(*this, engine)) {
			}
	};
}
