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

			for (const auto& [mat, mesh, mvp] : meshes) {
				cmd.material = mat.get();
				cmd.mesh = mesh.get();

				cmd.mvp = mvp;

				ctx.push(cmd);
			}
		}

		ctx.render();
	}
}
