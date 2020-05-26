#pragma once

// Meta
#include <Meta/TypeSet/TypeSet.hpp>

// Engine
#include <Engine/ECS/World.hpp>
#include <Engine/EngineInstance.hpp>

// Game
#include <Game/Common.hpp>

#include <Game/InputSystem.hpp>
#include <Game/ActionSystem.hpp>
#include <Game/CharacterMovementSystem.hpp>
#include <Game/PhysicsOriginShiftSystem.hpp>
#include <Game/PhysicsSystem.hpp>
#include <Game/CharacterSpellSystem.hpp>
#include <Game/CameraTrackingSystem.hpp>
#include <Game/SubWorldSystem.hpp>
#include <Game/MapSystem.hpp>
#include <Game/MapRenderSystem.hpp>
#include <Game/SpriteSystem.hpp>
#include <Game/NetworkingSystem.hpp>

#include <Game/PhysicsComponent.hpp>
#include <Game/SpriteComponent.hpp>
#include <Game/CharacterMovementComponent.hpp>
#include <Game/CharacterSpellComponent.hpp>
#include <Game/InputComponent.hpp>
#include <Game/ActionComponent.hpp>
#include <Game/PlayerComponent.hpp>
#include <Game/ConnectionComponent.hpp>
#include <Game/ConnectionStatsComponent.hpp>


namespace Game {
	using SystemsSet = Meta::TypeSet::TypeSet<
		InputSystem,
		ActionSystem,
		CharacterMovementSystem,
		PhysicsOriginShiftSystem,
		PhysicsSystem,
		CharacterSpellSystem,
		CameraTrackingSystem,
		//SubWorldSystem,
		MapSystem,
		MapRenderSystem,
		SpriteSystem,
		NetworkingSystem
	>;
	
	using ComponentsSet = Meta::TypeSet::TypeSet<
		MapEditComponent,
		PhysicsComponent,
		SpriteComponent,
		CharacterMovementComponent,
		CharacterSpellComponent,
		InputComponent,
		ActionComponent,
		PlayerComponent,
		ConnectionComponent,
		ConnectionStatsComponent
	>;

	class World : public Engine::ECS::World<tickrate, SystemsSet, ComponentsSet> {
		public:
			World(float tickInterval, Engine::EngineInstance& engine)
				: Engine::ECS::World<tickrate, SystemsSet, ComponentsSet>(tickInterval, std::tie(*this, engine)) {
			}
	};
}
