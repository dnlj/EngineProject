#pragma once

// Meta
#include <Meta/TypeSet/TypeSet.hpp>

// Engine
#include <Engine/ECS/World.hpp>
#include <Engine/EngineInstance.hpp>

// Game
#include <Game/Common.hpp>

#include <Game/systems/InputSystem.hpp>
#include <Game/systems/ActionSystem.hpp>
#include <Game/systems/CharacterMovementSystem.hpp>
#include <Game/systems/PhysicsOriginShiftSystem.hpp>
#include <Game/systems/PhysicsSystem.hpp>
#include <Game/systems/PhysicsInterpSystem.hpp>
#include <Game/systems/CharacterSpellSystem.hpp>
#include <Game/systems/CameraTrackingSystem.hpp>
#include <Game/systems/SubWorldSystem.hpp>
#include <Game/systems/MapSystem.hpp>
#include <Game/systems/MapRenderSystem.hpp>
#include <Game/systems/SpriteSystem.hpp>
#include <Game/systems/EntityNetworkingSystem.hpp>
#include <Game/systems/NetworkingSystem.hpp>
#include <Game/systems/UISystem.hpp>

#include <Game/comps/PhysicsBodyComponent.hpp>
#include <Game/comps/PhysicsInterpComponent.hpp>
#include <Game/comps/SpriteComponent.hpp>
#include <Game/comps/ActionComponent.hpp>
#include <Game/comps/ConnectionComponent.hpp>
#include <Game/comps/ECSNetworkingComponent.hpp>
#include <Game/comps/NetworkStatsComponent.hpp>
#include <Game/comps/MapAreaComponent.hpp>
#include <Game/comps/BlockEntityComponent.hpp>


namespace Game {
	using SystemsSet = Meta::TypeSet::TypeSet<
		// Networking
		EntityNetworkingSystem,
		NetworkingSystem,

		// Inputs
		InputSystem,
		ActionSystem,

		// Game Logic
		CharacterMovementSystem,
		PhysicsOriginShiftSystem,
		PhysicsSystem,
		CharacterSpellSystem,
		//SubWorldSystem,
		PhysicsInterpSystem,
		CameraTrackingSystem,
		MapSystem,

		// Rendering
		MapRenderSystem,
		SpriteSystem,
		UISystem
	>;
	
	using ComponentsSet = Meta::TypeSet::TypeSet<
		MapEditComponent,
		PhysicsBodyComponent,
		PhysicsInterpComponent,
		SpriteComponent,
		ActionComponent,
		ConnectionComponent,
		ECSNetworkingComponent,
		NetworkStatsComponent,
		MapAreaComponent,
		BlockEntityComponent,
		struct CharacterSpellComponent, // TODO: rn xFlag?
		struct PlayerFlag,
		struct CameraTargetFlag,
		struct NetworkedFlag
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
