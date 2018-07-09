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
	class SpriteSystem;
	class CharacterMovementSystem;
	class InputSystem;

	class PhysicsComponent;
	class RenderComponent;
	class SpriteComponent;
	class CharacterMovementComponent;
	class InputComponent;

	using SystemsSet = Meta::TypeSet::TypeSet<
		PhysicsSystem,
		RenderSystem,
		SpriteSystem,
		CharacterMovementSystem,
		InputSystem
	>;
	
	using ComponentsSet = Meta::TypeSet::TypeSet<
		PhysicsComponent,
		RenderComponent,
		SpriteComponent,
		CharacterMovementComponent,
		InputComponent
	>;

	using World = Engine::ECS::World<SystemsSet, ComponentsSet>;
	using SystemBase = Engine::SystemBase<World>;
}

// Game
#include <Game/PhysicsSystem.hpp>
#include <Game/RenderSystem.hpp>
#include <Game/SpriteSystem.hpp>
#include <Game/CharacterMovementSystem.hpp>
#include <Game/InputSystem.hpp>

#include <Game/PhysicsComponent.hpp>
#include <Game/RenderComponent.hpp>
#include <Game/SpriteComponent.hpp>
#include <Game/CharacterMovementComponent.hpp>
#include <Game/InputComponent.hpp>
