#pragma once

// Game
#include <Game/Terrain/Layer/BiomeFoo.hpp>

// TODO: Would be ideal to cleanup these includes so we only need the biomes we care about.
#include <Game/Terrain/TestGenerator.hpp>
#include <Game/Terrain/Generator.hpp>
#include <Game/Terrain/biomes/all.hpp>


namespace Game::Terrain::Layer {
	void BiomeFooHeight::request(const Range area, TestGenerator& generator) {
		generator.request<WorldBaseHeight>(area);
	}

	Float BiomeFooHeight::get(BIOME_HEIGHT_ARGS) const noexcept {
		return h0 + 15 * simplex.value(blockCoordX * 0.05_f, 0); // TODO: 1d simplex
	}

	void BiomeFooBasisStrength::request(const Range area, TestGenerator& generator) {
		// TODO: request verifier
	}

	Float BiomeFooBasisStrength::get(BIOME_BASIS_STRENGTH_ARGS) const noexcept {
		return 0.5_f + 0.5_f * simplex.value(glm::vec2{blockCoord} * 0.03_f);
	}
}
