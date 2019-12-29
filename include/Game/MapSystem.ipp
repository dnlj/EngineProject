#pragma once

// Game
#include <Game/MapSystem.hpp>

namespace Game {
	// TODO: Update to use map system conversion functions
	template<MapChunk::EditMemberFunction func>
	void MapSystem::applyEdit() {
		// TODO: do we need to updateOrigin()?
		const auto mpos = camera->screenToWorld(input->getMousePosition());

		// Offset of tile relative to current offset
		const auto offsetTile = glm::floor(mpos / MapChunk::tileSize);

		// Chunk position relative to current offset
		const glm::ivec2 offsetChunk = glm::floor(offsetTile / glm::vec2{MapChunk::size});

		// Absolute chunk position
		const auto absChunk = mapOffset * originRange + offsetChunk;

		// Index for this chunk
		const auto indexChunk = (mapSize + absChunk % mapSize) % mapSize;

		// Index of this tile in this chunk
		const auto indexTile = (MapChunk::size + glm::ivec2{offsetTile} % MapChunk::size) % MapChunk::size;

		MapChunk& chunk = chunks[indexChunk.x][indexChunk.y];

		(chunk.*func)(indexTile.x, indexTile.y);
		chunk.generate();
	}
}
