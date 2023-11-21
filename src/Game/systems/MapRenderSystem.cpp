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
#include <Game/systems/ZoneManagementSystem.hpp>
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

		const auto& plyFilter = world.getFilter<PlayerFlag, CameraTargetFlag, PhysicsBodyComponent>();
		if (plyFilter.size() == 0) { return; }

		if (ENGINE_DEBUG && plyFilter.size() != 1) [[unlikely]] {
			ENGINE_WARN2("Unexpected number of players {}", plyFilter.size());
		}

		// TODO: these should be part of model/mesh or maprendersystem. why are they on mapsystem
		const auto& mapSys = world.getSystem<MapSystem>();
		auto& shader = mapSys.shader;

		glUseProgram(shader->get());

		// Setup Texture
		glBindTextureUnit(0, mapSys.texArr.get());
		glUniform1i(4, 0);

		auto& cam = engine.getCamera();
		const auto vp = cam.getProjection() * cam.getView();

		const auto ply = plyFilter.front();
		const auto physComp = world.getComponent<PhysicsBodyComponent>(ply);
		const auto offset = world.getSystem<ZoneManagementSystem>().getZone(physComp.getZoneId()).offset2;
		const auto bounds = cam.getWorldScreenBounds();
		const auto minChunk = blockToChunk(worldToBlock2(bounds.min, offset));
		const auto maxChunk = blockToChunk(worldToBlock2(bounds.max, offset));

		const auto vao = mapSys.vertexLayout->get();
		glBindVertexArray(vao);

		for (auto x = minChunk.x; x <= maxChunk.x; ++x) {
			for (auto y = minChunk.y; y <= maxChunk.y; ++y) {
				const auto found = mapSys.activeChunks.find({x, y});
				if (found == mapSys.activeChunks.cend()) { continue; }
				const auto& data = found->second;
				if (data.ecount == 0) { continue; }

				const auto pos = data.body.getPosition();
				const auto mvp = glm::translate(vp, glm::vec3(pos.x, pos.y, 0.0f));
				glUniformMatrix4fv(0, 1, GL_FALSE, &mvp[0][0]);

				glVertexArrayVertexBuffer(vao, 0, data.vbuff.get(), 0, sizeof(MapSystem::Vertex));
				glVertexArrayElementBuffer(vao, data.ebuff.get());
				glDrawElements(GL_TRIANGLES, data.ecount, GL_UNSIGNED_SHORT, 0);
			}
		}
	}
}
