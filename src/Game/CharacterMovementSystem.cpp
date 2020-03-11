// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// GLFW
#include <GLFW/glfw3.h>

// Game
#include <Game/CharacterMovementSystem.hpp>
#include <Game/PhysicsComponent.hpp>
#include <Game/CharacterMovementComponent.hpp>
#include <Game/InputComponent.hpp>
#include <Game/World.hpp>


namespace Game {
	CharacterMovementSystem::CharacterMovementSystem(SystemArg arg)
		: System{arg}
		, filter{world.getFilterFor<
			Game::PhysicsComponent,
			Game::CharacterMovementComponent
		>()} {

		static_assert(World::orderBefore<CharacterMovementSystem, PhysicsSystem>());
	}

	void CharacterMovementSystem::run(float dt) {
		constexpr float speed = 1.0f * 4;
		for (auto ent : filter) {
			auto& physComp = world.getComponent<Game::PhysicsComponent>(ent);
			auto& moveComp = world.getComponent<Game::CharacterMovementComponent>(ent);

			if (moveComp.dir.x != 0 || moveComp.dir.y != 0) {
				physComp.body->ApplyLinearImpulseToCenter(
					dt * speed * b2Vec2{static_cast<float>(moveComp.dir.x), static_cast<float>(moveComp.dir.y)},
					true
				);
			}
		}
	}
}
