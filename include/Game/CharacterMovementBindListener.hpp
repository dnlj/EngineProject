#pragma once

// GLM
#include <glm/vec2.hpp>

// Engine
#include <Engine/BindPressListener.hpp>
#include <Engine/BindReleaseListener.hpp>

// Game
#include <Game/Common.hpp>


namespace Game {
	class CharacterMovementBindListener : public Engine::BindPressListener, public Engine::BindReleaseListener {
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
