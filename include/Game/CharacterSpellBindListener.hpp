#pragma once

// Engine
#include <Engine/EngineInstance.hpp>
#include <Engine/Input/BindPressListener.hpp>

// Game
#include <Game/World.hpp>

namespace Game {
	class CharacterSpellBindListener : public Engine::Input::BindPressListener {
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
			Engine::Input::AxisId axisIds[2];

			virtual void onBindPress() override;
	};
}
