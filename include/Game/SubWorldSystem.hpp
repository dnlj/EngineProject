#pragma once

// GLM
#include <glm/glm.hpp>

// Box2D
#include <box2d/b2_math.h>

// Engine
#include <Engine/ECS/EntityFilter.hpp>

// Game
#include <Game/System.hpp>
#include <Game/PhysicsComponent.hpp>


namespace Game {
	class SubWorldSystem : public System {
		public:
			// Somewhat arbitrary. Small enough to not loose precision. Large enough to not be constantly shifting.
			//constexpr static float32 range = 8000.0f; // Gives us .0005 precision (values < 8192)

			constexpr static float32 minRange = 32.0f; // TODO: rm - temp for testing. use above
			constexpr static float32 maxRange = 128.0f; // TODO: rm - temp for testing. use above
			constexpr static auto minRangeSquared = minRange * minRange;
			constexpr static auto maxRangeSquared = maxRange * maxRange;

		private:
			// TODO: reprecent this data with ecs
			struct Group {
				struct PlayerData* first;
				b2World* world;
				Engine::FlatHashMap<b2World*, int> worlds;
			};

			struct PlayerData {
				Engine::ECS::Entity ent;
				PhysicsComponent* physComp;
				b2Vec2 pos;
				b2World* world;
				Group* group = nullptr;
				bool shouldSplit = false;
			};
			std::vector<PlayerData> playerData;
			Engine::ECS::EntityFilter playerFilter;

		public:
			SubWorldSystem(SystemArg arg);

			void tick(float32 dt);

		private:
			void mergePlayer(PlayerData& ply);
			void splitFromWorld(PlayerData& ply);
	};
}
