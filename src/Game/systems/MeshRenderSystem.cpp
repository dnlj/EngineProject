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
			auto& modelComp = world.getComponent<ModelComponent>(ent);

			for (auto& data : modelComp.meshes) {
				cmd.material = data.mat.get();
				cmd.mesh = data.mesh.get();

				cmd.mvp = data.mvp;

				cmd.blockBindings.resize(data.bindings.size());
				for (int i = 0; i < data.bindings.size(); ++i) {
					auto& from = data.bindings[i];
					auto& to = cmd.blockBindings[i];
					to.buff = from.buff.get();
					to.index = from.index;
					to.offset = from.offset;
					to.size = from.size;
				}

				ctx.push(cmd);
			}
		}

		ctx.render();
	}
}
