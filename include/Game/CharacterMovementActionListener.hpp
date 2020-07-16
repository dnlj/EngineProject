#pragma once

// GLM
#include <glm/vec2.hpp>

// Game
#include <Game/World.hpp>


namespace Game {
	class CharacterMovementActionListener {
		public:
			Game::World& world;
			const glm::ivec2 move;

			bool operator()(Engine::ECS::Entity ent, Engine::Input::ActionId aid, Engine::Input::Value curr, Engine::Input::Value prev) {
				ENGINE_LOG("Mov: ", curr.value, " ", prev.value, " ", world.getTick(), " (", move.x, ", ", move.y, ")");

				if (curr && !prev) {
					auto& moveComp = world.getComponent<Game::CharacterMovementComponent>(ent);
					moveComp.dir += move;

					if (moveComp.dir.x > move.x || moveComp.dir.y > move.y) {
						ENGINE_LOG("Woopsies1");
					}
				} else {
					auto& moveComp = world.getComponent<Game::CharacterMovementComponent>(ent);
					moveComp.dir -= move;

					if (moveComp.dir.x > move.x || moveComp.dir.y > move.y) {
						ENGINE_LOG("Woopsies2");
					}
				}


				return false;
			};
	};
}
