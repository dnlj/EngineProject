// STD
#include <iterator>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// GLM
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Engine
#include <Engine/Utility/Utility.hpp>

// Game
#include <Game/MapChunk.hpp>
#include <Game/SpriteSystem.hpp>


namespace Game {
	MapChunk::MapChunk() {
	}

	MapChunk::~MapChunk() {
	}

	void MapChunk::from(const glm::vec2 wpos, const glm::ivec2 chunkPos) {
		pos = chunkPos;
	}

	glm::ivec2 MapChunk::getPosition() const {
		return pos;
	}
}
