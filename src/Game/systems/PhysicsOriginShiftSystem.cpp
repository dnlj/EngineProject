// Game
#include <Game/systems/PhysicsSystem.hpp>
#include <Game/systems/PhysicsOriginShiftSystem.hpp>
#include <Game/World.hpp>

/*
namespace Game {
	PhysicsOriginShiftSystem::PhysicsOriginShiftSystem(SystemArg arg)
		: System{arg} {
		static_assert(World::orderBefore<PhysicsOriginShiftSystem, PhysicsSystem>());
	}

	void PhysicsOriginShiftSystem::update(float32 dt) {
		return;
		// Using last frames position shouldnt be a problem here normally.
		// May be an issue in cases of long distance teleportation.
		const auto& pos = engine.getCamera().getPosition();

		if (std::abs(pos.x) > range) {
			std::cout
				<< "shift x "
				<< pos.x << " "
				<< range << "\n";
			auto& physSys = world.getSystem<Game::PhysicsSystem>();
			auto dir = std::copysign(1.0f, pos.x);

			physSys.shiftOrigin({
				range * dir,
				0.0f
			});

			offset.x += static_cast<int>(dir);
		}

		if (std::abs(pos.y) > range) {
			std::cout
				<< "shift y "
				<< pos.y << " "
				<< range << "\n";
			auto& physSys = world.getSystem<Game::PhysicsSystem>();
			auto dir = std::copysign(1.0f, pos.y);

			physSys.shiftOrigin({
				0.0f,
				range * dir
			});

			offset.y += static_cast<int>(dir);
		}
	}

	glm::ivec2 PhysicsOriginShiftSystem::getOffset() const {
		return offset;
	}
}
*/
