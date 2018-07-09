#pragma once

// Meta
#include <Meta/TypeSet/TypeSet.hpp>

// Engine
#include <Engine/ECS/World.hpp>
#include <Engine/SystemBase.hpp>

// TODO: split into files?
namespace Game {
	class PhysicsSystem;
	class SpriteSystem;
	class CharacterMovementSystem;
	class InputSystem;

	class PhysicsComponent;
	class SpriteComponent;
	class CharacterMovementComponent;
	class InputComponent;

	using SystemsSet = Meta::TypeSet::TypeSet<
		PhysicsSystem,
		SpriteSystem,
		CharacterMovementSystem,
		InputSystem
	>;
	
	using ComponentsSet = Meta::TypeSet::TypeSet<
		PhysicsComponent,
		SpriteComponent,
		CharacterMovementComponent,
		InputComponent
	>;

	using World = Engine::ECS::World<SystemsSet, ComponentsSet>;
	using SystemBase = Engine::SystemBase<World>;
}

// Game
#include <Game/PhysicsSystem.hpp>
#include <Game/SpriteSystem.hpp>
#include <Game/CharacterMovementSystem.hpp>
#include <Game/InputSystem.hpp>

#include <Game/PhysicsComponent.hpp>
#include <Game/SpriteComponent.hpp>
#include <Game/CharacterMovementComponent.hpp>
#include <Game/InputComponent.hpp>
