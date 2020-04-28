#pragma once

// GLM
#include <glm/vec2.hpp>

// Game
#include <Game/World.hpp>


namespace Game {
	class CharacterMovementBindListener {
		public:
			Game::World& world;
			const Engine::ECS::Entity player;
			const glm::ivec2 move;

			bool operator()(Engine::Input::Value curr, Engine::Input::Value prev) {
				if (curr && !prev) {
					auto& moveComp = world.getComponent<Game::CharacterMovementComponent>(player);
					moveComp.dir += move;
				} else {
					auto& moveComp = world.getComponent<Game::CharacterMovementComponent>(player);
					moveComp.dir -= move;
				}

				return false;
			};
	};
}
