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
				if (curr && !prev) {
					auto& moveComp = world.getComponent<Game::CharacterMovementComponent>(ent);
					moveComp.dir += move;
				} else {
					auto& moveComp = world.getComponent<Game::CharacterMovementComponent>(ent);
					moveComp.dir -= move;
				}

				return false;
			};
	};
}
