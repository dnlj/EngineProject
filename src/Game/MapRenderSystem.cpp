// STD
#include <algorithm>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Game
#include <Game/MapSystem.hpp>
#include <Game/MapRenderSystem.hpp>


namespace Game {
	MapRenderSystem::MapRenderSystem(World& world) : SystemBase{world} {
		priorityAfter = world.getBitsetForSystems<Game::MapSystem>();
	}

	MapRenderSystem::~MapRenderSystem() {
	}

	// TODO: this should probably be part of a more generic render system.
	void MapRenderSystem::run(float dt) {
		const auto& mapSys = world.getSystem<MapSystem>();
		// TODO: these should be part of model/mesh
		auto& shader = mapSys.shader;
		auto& texture = mapSys.texture;
		auto& camera = mapSys.camera;

		glUseProgram(shader.get());

		// Setup Texture
		glBindTextureUnit(0, texture.get());
		glUniform1i(5, 0);

		const auto vp = camera->getProjection() * camera->getView();

		// TODO: only draw on screen
		for (int x = 0; x < MapSystem::activeAreaSize.x; ++x) {
			for (int y = 0; y < MapSystem::activeAreaSize.y; ++y) {
				const auto& data = mapSys.activeAreaData[x][y];
				const auto pos = data.body->GetPosition();
				const auto mvp = glm::translate(vp, glm::vec3(pos.x, pos.y, 0.0f));

				glUniformMatrix4fv(1, 1, GL_FALSE, &mvp[0][0]);

				data.mesh.draw();
			}
		}
	}
}
