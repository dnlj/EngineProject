// Engine
#include <Engine/Glue/Box2D.hpp>
#include <Engine/Glue/glm.hpp>

// Game
#include <Game/CharacterSpellBindListener.hpp>
#include <Game/CharacterSpellSystem.hpp>
#include <Game/PhysicsComponent.hpp>


namespace Game {
	CharacterSpellBindListener::CharacterSpellBindListener(
		Engine::EngineInstance& engine,
		World& world,
		Engine::ECS::Entity player)
		: engine{engine}
		, world{world}
		, player{player}
		, targetIds{engine.actionManager.getId("Target_X", "Target_Y")} {
	}

	bool CharacterSpellBindListener::operator()(Engine::Input::Value curr, Engine::Input::Value prev) {
		if (!curr.value || prev.value) { return false; }

		auto& spellSys = world.getSystem<CharacterSpellSystem>();
		const auto& pos = world.getComponent<Game::PhysicsComponent>(player).getPosition();
		const auto mousePos = engine.camera.screenToWorld(engine.actionManager.getValue<float32>(targetIds));
		auto dir = Engine::Glue::as<b2Vec2>(mousePos) - pos;
		dir.Normalize();
		
		spellSys.fireMissile(pos + 0.3f * dir, dir);
		return false;
	}
}
