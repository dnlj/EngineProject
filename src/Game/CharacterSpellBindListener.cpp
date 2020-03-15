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
		, player{player} {
	}

	void CharacterSpellBindListener::onBindPress() {
		auto& spellSys = world.getSystem<CharacterSpellSystem>();
		const auto& pos = world.getComponent<Game::PhysicsComponent>(player).getPosition();
		auto mousePos = engine.camera.screenToWorld(engine.inputManager.getMousePosition());
		auto dir = b2Vec2(mousePos.x - pos.x, mousePos.y - pos.y);
		dir.Normalize();
		
		spellSys.fireMissile(pos + 0.3f * dir, dir);
	}
}
