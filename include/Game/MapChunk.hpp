#pragma once

// Game
#include <Game/MapTile.hpp>
#include <Game/PhysicsSystem.hpp>


namespace Game {
	class MapChunk {
		public:
			constexpr static MapTile AIR{0};
			constexpr static MapTile DIRT{1};

		public:
			constexpr static int width = 16;
			constexpr static int height = width;
			constexpr static auto tileSize = 1.0f/4.0f;

		private:
			int data[width][height] = {
				{3, 0, 0, 0, 0, 0, 0, 3},
				{0, 2, 0, 0, 0, 0, 2, 0},
				{0, 0, 2, 2, 2, 2, 0, 0},
				{0, 0, 2, 1, 1, 2, 0, 0},
				{0, 0, 2, 0, 1, 2, 0, 0},
				{0, 0, 2, 2, 2, 2, 0, 0},
				{0, 0, 0, 0, 0, 0, 2, 0},
				{4, 0, 0, 0, 0, 0, 0, 3},
			};

			b2Body* body = nullptr; // TODO: Cleanup
			Engine::ECS::Entity ent;

		public:
			void setup(World& world, glm::vec2 pos);
			void addTile(int x, int y, PhysicsSystem& physSys);
			void removeTile(int x, int y, PhysicsSystem& physSys);
			void generate(PhysicsSystem& physSys);
	};
}
