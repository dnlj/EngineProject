#pragma once

// Meta
#include <Meta/TypeSet/TypeSet.hpp>

// Engine
#include <Engine/ECS/World.hpp>
#include <Engine/SystemBase.hpp>


namespace Game {
	using namespace Engine::Types;

	using SystemsSet = Meta::TypeSet::TypeSet<
		class CharacterMovementSystem,
		class CharacterSpellSystem,
		class PhysicsSystem,
		class CameraTrackingSystem,
		class MapSystem,
		class MapRenderSystem,
		class SpriteSystem
	>;
	
	using ComponentsSet = Meta::TypeSet::TypeSet<
		class PhysicsComponent,
		class SpriteComponent,
		class CharacterMovementComponent,
		class CharacterSpellComponent,
		class InputComponent
	>;

	using World = Engine::ECS::World<SystemsSet, ComponentsSet>;
	using SystemBase = Engine::SystemBase<World>;
}
