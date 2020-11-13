// STD
#include <algorithm>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

	// TODO: this should probably be part of a more generic render system.
	void MapRenderSystem::run(float dt) {
		const auto& mapSys = world.getSystem<MapSystem>();
		// TODO: these should be part of model/mesh
		auto& shader = mapSys.shader;
		auto& texture = mapSys.texture;

		glUseProgram(shader.get());

		// Setup Texture
		glBindTextureUnit(0, texture.get());
		glUniform1i(5, 0);

		const auto vp = engine.camera.getProjection() * engine.camera.getView();

		// TODO: only draw on screen
		for (int x = 0; x < MapSystem::activeAreaSize.x; ++x) {
			for (int y = 0; y < MapSystem::activeAreaSize.y; ++y) {
				const auto& data = mapSys.activeAreaData[x][y];
				const auto pos = world.getComponent<PhysicsInterpComponent>(data.ent).getPosition();
				const auto mvp = glm::translate(vp, glm::vec3(pos.x, pos.y, 0.0f));

				glUniformMatrix4fv(1, 1, GL_FALSE, &mvp[0][0]);

				data.mesh.draw();
			}
		}
	}
}
