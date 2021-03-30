// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// Game
#include <Game/World.hpp>

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
	}

	void CharacterMovementSystem::tick() {
		constexpr float speed = 1.0f * 200;

		for (auto ent : world.getFilter<Filter>()) {
			auto& physComp = world.getComponent<PhysicsBodyComponent>(ent);
			const auto& actComp = world.getComponent<ActionComponent>(ent);
			// TODO: should we use press count here?
			const bool up = actComp.getButton(Button::MoveUp).latest;
			const bool down = actComp.getButton(Button::MoveDown).latest;
			const bool left = actComp.getButton(Button::MoveLeft).latest;
			const bool right = actComp.getButton(Button::MoveRight).latest;
			const b2Vec2 move = {static_cast<float32>(right - left), static_cast<float32>(up - down)};

			if (move != b2Vec2_zero) {
				physComp.getBody().ApplyLinearImpulseToCenter(world.getTickDelta() * speed * move, true);
			}
		}
	}
}
