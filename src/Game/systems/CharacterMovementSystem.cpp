// Box2D
#include <box2d/b2_math.h>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// Game
#include <Game/World.hpp>
#include <Game/systems/CharacterMovementSystem.hpp>
#include <Game/comps/ActionComponent.hpp>
#include <Game/comps/PhysicsBodyComponent.hpp>


namespace {
	using Filter = Engine::ECS::EntityFilterList<
			Game::PhysicsBodyComponent,
			Game::ActionComponent
	>;
}

namespace Game {
	CharacterMovementSystem::CharacterMovementSystem(SystemArg arg)
		: System{arg} {

		static_assert(World::orderBefore<CharacterMovementSystem, PhysicsSystem>());
		static_assert(World::orderAfter<CharacterMovementSystem, ActionSystem>());
	}

	void CharacterMovementSystem::tick() {
		constexpr float speed = 1.0f * 50;

		for (auto ent : world.getFilter<Filter>()) {
			auto& physComp = world.getComponent<PhysicsBodyComponent>(ent);
			const auto& actComp = world.getComponent<ActionComponent>(ent);
			// TODO: should we use press count here?
			const bool up = actComp.getAction(Action::MoveUp).latest;
			const bool down = actComp.getAction(Action::MoveDown).latest;
			const bool left = actComp.getAction(Action::MoveLeft).latest;
			const bool right = actComp.getAction(Action::MoveRight).latest;
			const b2Vec2 move = {static_cast<float32>(right - left), static_cast<float32>(up - down)};

			if (move != b2Vec2_zero) {
				physComp.applyLinearImpulse(world.getTickDelta() * speed * move, true);
			}

			// TODO: disable outside of debug/cvar cheats/etc.
			if (true) {
				const auto dirX = 1.0f*actComp.getAction(Action::DebugTeleRight).pressCount - actComp.getAction(Action::DebugTeleLeft).pressCount;
				const auto dirY = 1.0f*actComp.getAction(Action::DebugTeleUp).pressCount - actComp.getAction(Action::DebugTeleDown).pressCount;
				if (dirX || dirY) { physComp.setPosition(physComp.getPosition() + 100.0f * b2Vec2{dirX, dirY}); }
			}
		}
	}
}
