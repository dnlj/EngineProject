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
		class ModelComponent
	>;

	using FlagsSet = Meta::TypeSet::TypeSet<
		struct CharacterSpellComponent, // TODO: rn xFlag
		struct PlayerFlag,
		struct CameraTargetFlag,
		struct NetworkedFlag
	>;

	// TODO is there a good reason we dont just have a `using=` decl here?
	class World : public Engine::ECS::WorldHelper<tickrate, SystemsSet, CompsSet, FlagsSet> {
		public:
			World(EngineInstance& engine)
				: Engine::ECS::WorldHelper<tickrate, SystemsSet, CompsSet, FlagsSet>(std::tie(*this, engine)) {
			}
	};

	using ComponentsSet = World::ComponentsSetType;



	// TODO: move this somewhere?
	template<class T>
	concept IsNetworkedComponent_internal = requires (T t) {
		Engine::Net::Replication{t.netRepl()};
	};

	template<class T>
	constexpr bool checkIsNetworkedComponent() {
		if constexpr (World::IsFlagComponent<T>::value) {
			return false;
		} else {
			static_assert(World::IsNonFlagComponent<T>::value, "Attempting to check if an invalid component should be networked.");
			static_assert(Engine::Meta::IsComplete<T>::value, "Attempting to check if an incomplete component should be networked.");
			return IsNetworkedComponent_internal<T>;
		}
	}

	template<class T>
	concept IsNetworkedComponent = checkIsNetworkedComponent<T>();
}
