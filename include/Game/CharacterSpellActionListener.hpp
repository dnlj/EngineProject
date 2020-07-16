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

		public:
			CharacterSpellActionListener::CharacterSpellActionListener(
				Engine::EngineInstance& engine,
				World& world)
				: engine{engine}
				, world{world} {
			}

			bool CharacterSpellActionListener::operator()(Engine::ECS::Entity ent, Engine::Input::ActionId aid, Engine::Input::Value curr, Engine::Input::Value prev) {
				if (!curr.value || prev.value) { return false; }

				// TODO: we shouldnt actually need this once we get syncing correct.
				if (!world.hasComponents<PhysicsComponent, ActionComponent>(ent)) { return false; }

				auto& [physComp, actComp] = world.getComponents<PhysicsComponent, ActionComponent>(ent);
				const auto& pos = physComp.getPosition();
				const glm::vec2 mousePos = {actComp.getAxis(Axis::TargetX), actComp.getAxis(Axis::TargetY)};
				auto dir = Engine::Glue::as<b2Vec2>(mousePos) - pos;
				dir.Normalize();
		
				auto& spellSys = world.getSystem<CharacterSpellSystem>();
				spellSys.fireMissile(pos + 0.3f * dir, dir);
				return false;
			}
	};
}
