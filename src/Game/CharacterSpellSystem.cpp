// Game
#include <Game/CharacterSpellSystem.hpp>

namespace Game {
	CharacterSpellSystem::CharacterSpellSystem(World& world)
		: SystemBase{world}
		, filter{world.getFilterFor<
			Game::CharacterSpellComponent,
			Game::InputComponent>()} {

		priorityAfter = world.getBitsetForSystems<Game::CharacterMovementSystem>();
		priorityBefore = world.getBitsetForSystems<Game::PhysicsSystem>();
	}

	void CharacterSpellSystem::run(float dt) {
		for (auto ent : filter) {
			auto& inputManager = *world.getComponent<Game::InputComponent>(ent).inputManager;

			if (inputManager.wasPressed("Spell_1")) {
				std::cout << "Spell 1\n";
			}
		}
	}
}
