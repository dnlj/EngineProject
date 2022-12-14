#pragma once

// Meta
#include <Meta/TypeSet/TypeSet.hpp>
#include <Engine/Meta/IsComplete.hpp>

// Engine
#include <Engine/ECS/World.hpp>
#include <Engine/Net/Replication.hpp>

// Game
#include <Game/Common.hpp>
#include <Game/EngineInstance.hpp>
#include <Game/NetworkTraits.hpp>


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
		class AnimSystem,
		class SpriteSystem,
		class UISystem,
		class MeshRenderSystem,
		class RenderPassSystem
	>;
	
	using CompsSet = Meta::TypeSet::TypeSet<
		class MapEditComponent,
		class PhysicsBodyComponent,
		class PhysicsInterpComponent,
		class SpriteComponent,
		class ActionComponent,
		class ConnectionComponent,
		class ECSNetworkingComponent,
		class NetworkStatsComponent,
		class MapAreaComponent,
		class BlockEntityComponent,
		class ModelComponent,
		class ArmatureComponent,
		class AnimationComponent
	>;

	using FlagsSet = Meta::TypeSet::TypeSet<
		struct PlayerFlag,
		struct CameraTargetFlag,
		struct NetworkedFlag
	>;

	// Example: template<> constexpr static inline bool IsNetworkedFlag<PlayerFlag> = true;
	//template<> constexpr static inline bool IsNetworkedFlag<PlayerFlag> = true;
	//template<> constexpr static inline bool IsNetworkedFlag<NetworkedFlag> = true;
	//template<> constexpr static inline bool IsNetworkedFlag<CameraTargetFlag> = true;

	// TODO is there a good reason we dont just have a `using=` decl here?
	class World : public Engine::ECS::WorldHelper<tickrate, SystemsSet, CompsSet, FlagsSet> {
		public:
			World(EngineInstance& engine)
				: Engine::ECS::WorldHelper<tickrate, SystemsSet, CompsSet, FlagsSet>(std::tie(*this, engine)) {
			}
	};

	using ComponentsSet = World::ComponentsSetType;
}
