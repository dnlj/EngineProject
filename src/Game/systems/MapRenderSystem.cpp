// STD
#include <algorithm>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Engine
#include <Engine/Gfx/Shader.hpp>

// Game
#include <Game/systems/MapSystem.hpp>
#include <Game/systems/MapRenderSystem.hpp>
#include <Game/World.hpp>


namespace Game {
	MapRenderSystem::MapRenderSystem(SystemArg arg) : System{arg} {
		static_assert(World::orderAfter<MapRenderSystem, MapSystem>());
	}

	MapRenderSystem::~MapRenderSystem() {
	}

	void MapRenderSystem::update(float dt) {
	}

	void MapRenderSystem::render(const RenderLayer layer) {
		if (layer != RenderLayer::Terrain) { return; }
		const auto& mapSys = world.getSystem<MapSystem>();
		// TODO: these should be part of model/mesh or maprendersystem. why are they on mapsystem
		auto& shader = mapSys.shader;

		glUseProgram(shader->get());

		// Setup Texture
		glBindTextureUnit(0, mapSys.texArr.get());
		glUniform1i(4, 0);

		auto& cam = engine.getCamera();
		const auto vp = cam.getProjection() * cam.getView();

		const auto bounds = cam.getWorldScreenBounds();
		const auto minChunk = MapSystem::blockToChunk(mapSys.worldToBlock(bounds.min));
		const auto maxChunk = MapSystem::blockToChunk(mapSys.worldToBlock(bounds.max));

		for (auto x = minChunk.x; x <= maxChunk.x; ++x) {
			for (auto y = minChunk.y; y <= maxChunk.y; ++y) {
				const auto found = mapSys.activeChunks.find({x, y});
				if (found == mapSys.activeChunks.cend()) { continue; }

				const auto pos = found->second.body->GetPosition();
				const auto mvp = glm::translate(vp, glm::vec3(pos.x, pos.y, 0.0f));
				glUniformMatrix4fv(0, 1, GL_FALSE, &mvp[0][0]);
				found->second.mesh.draw();
			}
		}
	}
}
