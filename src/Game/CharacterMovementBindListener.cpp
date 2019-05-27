// Game
#include <Game/CharacterMovementBindListener.hpp>
#include <Game/CharacterMovementComponent.hpp>

namespace Game {
	CharacterMovementBindListener::CharacterMovementBindListener(Game::World& world, Engine::ECS::Entity player, glm::ivec2&& move)
		: world{world}, player{player}, move{move} {
	}

	void CharacterMovementBindListener::onBindPress() {
		auto& moveComp = world.getComponent<Game::CharacterMovementComponent>(player);
		moveComp.dir += move;
	}

	void CharacterMovementBindListener::onBindRelease() {
		auto& moveComp = world.getComponent<Game::CharacterMovementComponent>(player);
		moveComp.dir -= move;
	}
}
