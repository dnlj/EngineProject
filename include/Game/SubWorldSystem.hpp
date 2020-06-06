#pragma once

// GLM
#include <glm/glm.hpp>

// Box2D
#include <box2d/b2_math.h>

// Game
#include <Game/System.hpp>
#include <Game/EntityFilter.hpp>
#include <Game/PhysicsComponent.hpp>
#include <Game/MapSystem.hpp>


namespace Game {
	class SubWorldSystem : public System {
		public:
			// Somewhat arbitrary. Small enough to not loose precision. Large enough to not be constantly shifting.
			//constexpr static float32 range = 8000.0f; // Gives us .0005 precision (values < 8192)

			constexpr static float32 minRange = 16.0f; // TODO: rm - temp for testing. use above
			constexpr static float32 maxRange = 32.0f; // TODO: rm - temp for testing. use above
			constexpr static auto minRangeSquared = minRange * minRange;
			constexpr static auto maxRangeSquared = maxRange * maxRange;

		private:
			// TODO: reprecent this data with ecs
			struct Group {
				b2World* world;
				Engine::FlatHashMap<b2World*, int> worlds;
			};

			struct PlayerData {
				Engine::ECS::Entity ent;
				PhysicsComponent* physComp;
				b2Vec2 pos;
				Group* group = nullptr;
				bool shouldSplit = false;
			};
			std::vector<PlayerData> playerData;
			std::vector<std::unique_ptr<b2World>> worlds; // TODO: who should own worlds? physics sys?
			std::vector<b2World*> freeWorlds;

			EntityFilter& playerFilter;

			std::vector<Group> groups;
			robin_hood::unordered_flat_set<b2Body*> bodies; // TODO: add engine type

		public:
			SubWorldSystem(SystemArg arg);

			void tick(float32 dt);

		private:
			void mergePlayer(PlayerData& ply, b2World* world);
			void splitFromWorld(PlayerData& ply);

			b2World* getFreeWorld();
	};
}
