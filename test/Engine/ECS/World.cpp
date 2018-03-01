// Engine
#include <Engine/ECS/World.hpp>

// GoogleTest
#include <gtest/gtest.h>

namespace {
	class WorldTest : public testing::Test {
		public:
			WorldTest() {
			}

			Engine::ECS::World world;
	};
}

TEST_F(WorldTest, Test1) {
}
