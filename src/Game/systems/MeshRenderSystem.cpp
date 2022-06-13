// Engine
#include <Engine/Gfx/DrawCommand.hpp>
#include <Engine/Gfx/Mesh2.hpp>
#include <Engine/Gfx/Shader.hpp>
#include <Engine/Gfx/VertexAttributeLayout.hpp>
#include <Engine/Gfx/Buffer.hpp>
#include <Engine/Gfx/Context.hpp>
#include <Engine/Gfx/Material.hpp>

// Game
#include <Game/systems/MeshRenderSystem.hpp>
#include <Game/comps/PhysicsInterpComponent.hpp>
#include <Game/comps/ModelComponent.hpp>


namespace Game {
	MeshRenderSystem::MeshRenderSystem(SystemArg arg) : System{arg} {
	}

	void MeshRenderSystem::render(RenderLayer layer) {
		if (layer != RenderLayer::Debug) { return; } // TODO: really this should be after everything else
		auto& ctx = engine.getGraphicsContext();

		Engine::Gfx::DrawCommand cmd;
		const auto& filter = world.getFilter<ModelComponent>();

		for (auto ent : filter) {
			const auto& [meshes] = world.getComponent<ModelComponent>(ent);

			for (const auto& [mat, mesh, mvp, params] : meshes) {
				cmd.shader = mat->getShader();
				cmd.vao = mesh->layout->vao;
				cmd.vbo = mesh->vbuff->get();
				cmd.vboStride = mesh->vstride;
				cmd.ebo = mesh->ebuff->get();
				cmd.ecount = mesh->ecount;
				cmd.eoffset = mesh->eoffset;

				cmd.mvp = mvp;

				cmd.params = &params;

				ctx.push(cmd);
			}
		}

		ctx.render();
	}
}
