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
		, axisIds{engine.inputManager.getAxisId("target_x"), engine.inputManager.getAxisId("target_y")} {
	}

	void CharacterSpellBindListener::onBindPress() {
		auto& spellSys = world.getSystem<CharacterSpellSystem>();
		const auto& pos = world.getComponent<Game::PhysicsComponent>(player).getPosition();
		const auto mousePos = engine.camera.screenToWorld(engine.inputManager.getAxisValue(axisIds));
		auto dir = Engine::Glue::as<b2Vec2>(mousePos) - pos;
		dir.Normalize();
		
		spellSys.fireMissile(pos + 0.3f * dir, dir);
	}
}
