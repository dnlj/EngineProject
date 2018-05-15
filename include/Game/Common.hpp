#pragma once

// Meta
#include <Meta/TypeSet/TypeSet.hpp>

// Engine
#include <Engine/ECS/World.hpp>
#include <Engine/SystemBase.hpp>

// TODO: split into files?
namespace Game {
	class PhysicsSystem;
	class RenderSystem;
	class CharacterMovementSystem;
	class PhysicsComponent;
	class RenderComponent;

	using SystemsSet = Meta::TypeSet::TypeSet<
		PhysicsSystem,
		RenderSystem,
		CharacterMovementSystem
	>;
	
	using ComponentsSet = Meta::TypeSet::TypeSet<
		PhysicsComponent,
		RenderComponent
	>;

	using World = Engine::ECS::World<SystemsSet, ComponentsSet>;
	using SystemBase = Engine::SystemBase<World>;
}

// Game
#include <Game/PhysicsSystem.hpp>
#include <Game/RenderSystem.hpp>
#include <Game/CharacterMovementSystem.hpp>
#include <Game/PhysicsComponent.hpp>
#include <Game/RenderComponent.hpp>
