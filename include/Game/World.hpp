#pragma once

// Meta
#include <Meta/TypeSet/TypeSet.hpp>

// Engine
#include <Engine/ECS/World.hpp>

// Game
#include <Game/Common.hpp>

#include <Game/comps/PhysicsBodyComponent.hpp>
#include <Game/comps/PhysicsInterpComponent.hpp>
#include <Game/comps/SpriteComponent.hpp>
#include <Game/comps/ActionComponent.hpp>
#include <Game/comps/ConnectionComponent.hpp>
#include <Game/comps/ECSNetworkingComponent.hpp>
#include <Game/comps/NetworkStatsComponent.hpp>
#include <Game/comps/MapAreaComponent.hpp>
#include <Game/comps/BlockEntityComponent.hpp>
#include <Game/comps/MapEditComponent.hpp>


namespace Game {
	using SystemsSet = Meta::TypeSet::TypeSet<
		// Networking
		class EntityNetworkingSystem,
		class NetworkingSystem,

		// Inputs
		class InputSystem,
		class ActionSystem,

		// Game Logic
		class CharacterMovementSystem,
		class PhysicsOriginShiftSystem,
		class PhysicsSystem,
		class CharacterSpellSystem,
		//SubWorldSystem,
		class PhysicsInterpSystem,
		class CameraTrackingSystem,
		class MapSystem,

		// Rendering
		//ParallaxBackgroundSystem,
		class MapRenderSystem,
		class SpriteSystem,
		class UISystem,
		class RenderPassSystem
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
			World(EngineInstance& engine)
				: Engine::ECS::World<World, tickrate, SystemsSet, ComponentsSet>(std::tie(*this, engine)) {
			}
	};
}
