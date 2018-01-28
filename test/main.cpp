// GLM
#include <glm/glm.hpp>

// Google Test
#include <gtest/gtest.h>

// TODO: Remove. These are temporary so we can compile until we make these not global.
namespace Game {
	glm::mat4 projection;
	glm::mat4 view;
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}