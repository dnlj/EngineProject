#pragma once

// Engine
#include <Engine/EngineInstance.hpp>

// Game
#include <Game/World.hpp>

namespace Game {
	class CharacterSpellBindListener {
		private:
			Engine::EngineInstance& engine;
			World& world;
			const Engine::ECS::Entity player;
			std::array<Engine::Input::ActionId, 2> targetIds;

		public:
			CharacterSpellBindListener(
				Engine::EngineInstance& engine,
				World& world,
				Engine::ECS::Entity player
			);

			bool operator()(Engine::Input::Value curr, Engine::Input::Value prev);
	};
}
