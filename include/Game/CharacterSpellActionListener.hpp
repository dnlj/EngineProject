#pragma once

// Engine
#include <Engine/EngineInstance.hpp>
#include <Engine/Glue/Box2D.hpp>

// Game
#include <Game/World.hpp>

namespace Game {
	class CharacterSpellActionListener {
		private:
			Engine::EngineInstance& engine;
			World& world;
			std::array<Engine::Input::ActionId, 2> targetIds;

		public:
			CharacterSpellActionListener::CharacterSpellActionListener(
				Engine::EngineInstance& engine,
				World& world)
				: engine{engine}
				, world{world}
				, targetIds{world.getSystem<ActionSystem>().getId("Target_X", "Target_Y")} {
			}

			bool CharacterSpellActionListener::operator()(Engine::ECS::Entity ent, Engine::Input::ActionId aid, Engine::Input::Value curr, Engine::Input::Value prev) {
				if (!curr.value || prev.value) { return false; }

				auto& spellSys = world.getSystem<CharacterSpellSystem>();
				auto& actC = world.getComponent<ActionComponent>(ent);
				const auto& pos = world.getComponent<Game::PhysicsComponent>(ent).getPosition();
				const auto mousePos = actC.getValue<float32>(targetIds);
				auto dir = Engine::Glue::as<b2Vec2>(mousePos) - pos;
				dir.Normalize();
		
				spellSys.fireMissile(pos + 0.3f * dir, dir);
				return false;
			}
	};
}
