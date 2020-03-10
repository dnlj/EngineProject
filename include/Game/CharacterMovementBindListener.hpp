#pragma once

// GLM
#include <glm/vec2.hpp>

// Engine
#include <Engine/Input/BindPressListener.hpp>
#include <Engine/Input/BindReleaseListener.hpp>

// Game
#include <Game/World.hpp>


namespace Game {
	class CharacterMovementBindListener : public Engine::Input::BindPressListener, public Engine::Input::BindReleaseListener {
		public:
			CharacterMovementBindListener(Game::World& world, Engine::ECS::Entity player, glm::ivec2&& move);

		private:
			Game::World& world;
			const Engine::ECS::Entity player;
			const glm::ivec2 move;

			virtual void onBindPress() override;
			virtual void onBindRelease() override;
	};
}
