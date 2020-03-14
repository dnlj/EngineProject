// Game
#include <Game/PhysicsOriginShiftSystem.hpp>
#include <Game/World.hpp>


namespace Game {
	PhysicsOriginShiftSystem::PhysicsOriginShiftSystem(SystemArg arg)
		: System{arg} {
		static_assert(World::orderBefore<PhysicsOriginShiftSystem, PhysicsSystem>());
	}

	void PhysicsOriginShiftSystem::run(float32 dt) {
		// TODO: interpolation causes an error with origin shifting

		// Using last frames position shouldnt be a problem here normally.
		// May be an issue in cases of long distance teleportation.
		const auto& pos = engine.camera.getPosition();

		if (std::abs(pos.x) > range) {
			std::cout
				<< "shift x "
				<< pos.x << " "
				<< range << "\n";
			auto& physSys = world.getSystem<Game::PhysicsSystem>();
			auto dir = std::copysign(1.0f, pos.x);

			physSys.getWorld().ShiftOrigin(b2Vec2{
				range * dir,
				0.0f
			});

			offset.x += static_cast<int>(dir);
		}

		if (std::abs(pos.y) > range) {
			std::cout
				<< "shift x "
				<< pos.x << " "
				<< range << "\n";
			auto& physSys = world.getSystem<Game::PhysicsSystem>();
			auto dir = std::copysign(1.0f, pos.y);

			physSys.getWorld().ShiftOrigin(b2Vec2{
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
