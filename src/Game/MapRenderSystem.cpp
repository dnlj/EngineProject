// Game
#include <Game/MapRenderSystem.hpp>


namespace Game {
	MapRenderSystem::MapRenderSystem(World& world) : SystemBase{world} {
		priorityAfter = world.getBitsetForSystems<Game::PhysicsSystem, Game::CameraTrackingSystem>();
	}

	void MapRenderSystem::run(float dt) {
	}
}
