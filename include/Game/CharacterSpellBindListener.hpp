#pragma once

// Engine
#include <Engine/EngineInstance.hpp>
#include <Engine/BindPressListener.hpp>

// Game
#include <Game/Common.hpp>

namespace Game {
	class CharacterSpellBindListener : public Engine::BindPressListener {
		public:
			CharacterSpellBindListener(
				Engine::EngineInstance& engine,
				World& world,
				Engine::ECS::Entity player
			);

		private:
			Engine::EngineInstance& engine;
			World& world;
			const Engine::ECS::Entity player;

			virtual void onBindPress() override;
	};
}
