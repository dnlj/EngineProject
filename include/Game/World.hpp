#pragma once

// Meta
#include <Meta/TypeSet/TypeSet.hpp>
#include <Engine/Meta/IsComplete.hpp>

// Engine
#include <Engine/ECS/World.hpp>
#include <Engine/Net/Replication.hpp>

// Game
#include <Game/common.hpp>
#include <Game/EngineInstance.hpp>
#include <Game/NetworkTraits.hpp>


namespace Game {
	using SystemsSet = Meta::TypeSet::TypeSet<
		// Networking & State Updates
		// Must be before physics + rendering. Updating between causes a flash/flick.
		class EntityNetworkingSystem,
		class NetworkingSystem,
		ENGINE_SERVER_ONLY(class RealmManagementSystem,)
		class ZoneManagementSystem,

		// Inputs
		class InputSystem,
		class ActionSystem,

		// Game Logic
		class CharacterMovementSystem,
		class PhysicsSystem,
		class CharacterSpellSystem,
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
		class ActionComponent,
		class AnimationComponent,
		class ArmatureComponent,
		class BlockEntityComponent,
		class ECSNetworkingComponent,
		class MapAreaComponent,
		class ModelComponent,
		class NetworkComponent,
		class NetworkStatsComponent,
		class PhysicsBodyComponent,
		class PhysicsInterpComponent,
		class RealmComponent,
		class SpriteComponent
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

	// TODO is there a good reason we dont just have a `using=` decl here? Forward decl issues? If so, doc.
	class World : public Engine::ECS::WorldHelper<tickrate, SystemsSet, CompsSet, FlagsSet> {
		public:
			World(EngineInstance& engine)
				: Engine::ECS::WorldHelper<tickrate, SystemsSet, CompsSet, FlagsSet>(std::tie(*this, engine)) {
			}
	};

	using ComponentsSet = World::ComponentsSetType;
}
