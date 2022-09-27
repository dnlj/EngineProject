// Engine
#include <Engine/Gfx/DrawCommand.hpp>
#include <Engine/Gfx/Mesh2.hpp>
#include <Engine/Gfx/Shader.hpp>
#include <Engine/Gfx/VertexAttributeLayout.hpp>
#include <Engine/Gfx/Buffer.hpp>
#include <Engine/Gfx/Context.hpp>
#include <Engine/Gfx/Material.hpp>

// Game
#include <Game/comps/ModelComponent.hpp>
#include <Game/systems/MeshRenderSystem.hpp>


namespace Game {
	MeshRenderSystem::MeshRenderSystem(SystemArg arg) : System{arg} {
	}

	void MeshRenderSystem::render(RenderLayer layer) {
		using namespace Engine::Gfx;

		if (layer != RenderLayer::Debug) { return; }
		auto& ctx = engine.getGraphicsContext();

		DrawCommand cmd;
		const auto& filter = world.getFilter<ModelComponent>();

		for (auto ent : filter) {
			auto& modelComp = world.getComponent<ModelComponent>(ent);

			for (auto& data : modelComp.meshes) {
				cmd.baseInstance = data.baseInstance;
				cmd.material = data.mat.get();
				cmd.mesh = data.mesh.get();

				// TODO: make a conversion operator
				cmd.uboBindings.resize(data.uboBindings.size());
				for (int i = 0; i < data.uboBindings.size(); ++i) {
					auto& from = data.uboBindings[i];
					auto& to = cmd.uboBindings[i];
					to.buff = from.buff.get();
					to.index = from.index;
					to.offset = from.offset;
					to.size = from.size;
				}

				// TODO: make a conversion operator
				cmd.vboBindings.resize(data.vboBindings.size());
				for (int i = 0; i < data.vboBindings.size(); ++i) {
					auto& from = data.vboBindings[i];
					auto& to = cmd.vboBindings[i];
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
