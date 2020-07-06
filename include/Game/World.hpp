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
#include <Game/UISystem.hpp>

#include <Game/PhysicsComponent.hpp>
#include <Game/SpriteComponent.hpp>
#include <Game/CharacterMovementComponent.hpp>
#include <Game/ActionComponent.hpp>
#include <Game/ConnectionComponent.hpp>
#include <Game/NeighborsComponent.hpp>


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
		NetworkingSystem,
		UISystem
	>;
	
	using ComponentsSet = Meta::TypeSet::TypeSet<
		MapEditComponent,
		PhysicsComponent,
		SpriteComponent,
		CharacterMovementComponent,
		ActionComponent,
		ConnectionComponent,
		NeighborsComponent,
		struct CharacterSpellComponent, // TODO: rn xFlag?
		struct PlayerFlag
	>;

	// TODO: we could get rid of CRTP here by forward declaring all systems/components/flags...
	// TODO: cont. - (effectively forward decl the sets) and making World a typedef and templating the ECS::World constructor...
	// TODO: cont. - Would need to move the set defs into own file then. Not sure if worth. Probably is. CRTP is a little stinky.
	class World : public Engine::ECS::World<World, tickrate, SystemsSet, ComponentsSet> {
		public:
			World(Engine::EngineInstance& engine)
				: Engine::ECS::World<World, tickrate, SystemsSet, ComponentsSet>(std::tie(*this, engine)) {
			}
	};
}
