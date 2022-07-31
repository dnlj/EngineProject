// Engine
#include <Engine/Camera.hpp>
#include <Engine/Gfx/BufferManager.hpp>
#include <Engine/Gfx/MaterialInstanceManager.hpp>
#include <Engine/Gfx/MaterialManager.hpp>
#include <Engine/Gfx/Mesh.hpp>
#include <Engine/Gfx/MeshManager.hpp>
#include <Engine/Gfx/ModelLoader.hpp>
#include <Engine/Gfx/ShaderLoader.hpp>
#include <Engine/Gfx/VertexAttributeLayout.hpp>
#include <Engine/Gfx/VertexLayoutLoader.hpp>

// Game
#include <Game/systems/AnimSystem.hpp>
#include <Game/comps/ModelComponent.hpp>
#include <Game/comps/ArmatureComponent.hpp>


namespace Game {
	AnimSystem::AnimSystem(SystemArg arg) : System{arg} {
		using namespace Engine::Gfx;

		ent = world.createEntity();
		auto& mdlComp = world.addComponent<ModelComponent>(ent);
		auto& armComp = world.addComponent<ArmatureComponent>(ent);

		{
			VertexAttributeDesc attribs[] = {
				{ VertexInput::Position, 3, NumberType::Float32, offsetof(Vertex, pos), false },
				{ VertexInput::BoneIndices, 4, NumberType::UInt8, offsetof(Vertex, bones), false },
				{ VertexInput::BoneWeights, 4, NumberType::Float32, offsetof(Vertex, weights), false },
			};

			auto layout = engine.getVertexLayoutLoader().get(attribs);

			// TODO: we probably also want a ModelLoader/Manager to cache this stuff so we dont load the same thing multiple times
			ModelLoader loader;
			skinned = !loader.arm.boneOffsets.empty();

			{
				const auto shader = engine.getShaderLoader().get(skinned ? "shaders/mesh" : "shaders/mesh_static");
				auto matBase = engine.getMaterialManager().create(shader);

				// TODO: load from model
				mats[0] = engine.getMaterialInstanceManager().create(matBase);
				mats[0]->set("color", glm::vec4{1,1,0.5,1});

				mats[1] = engine.getMaterialInstanceManager().create(matBase);
				mats[1]->set("color", glm::vec4{1,0.5,1,1});

				mats[2] = engine.getMaterialInstanceManager().create(matBase);
				mats[2]->set("color", glm::vec4{0.5,1,1,1});
			}

			ENGINE_INFO("**** Loaded Model: ", loader.verts.size(), " ", loader.indices.size(), " ", loader.instances.size(), " ", skinned);

			const auto vbo = engine.getBufferManager().create(loader.verts);
			const auto ebo = engine.getBufferManager().create(loader.indices);

			struct MeshInfo {
				MeshRef mesh;
				MaterialInstanceRef mat;
			};
			std::vector<MeshInfo> meshInfo;
			meshInfo.reserve(loader.meshes.size());

			for (auto& m : loader.meshes) {
				ENGINE_LOG("MeshDesc::material = ", m.material);
				meshInfo.emplace_back(
					engine.getMeshManager().create(
						layout,
						vbo, static_cast<uint32>(sizeof(Vertex)),
						ebo, m.offset, m.count
					),
					mats[m.material] // TODO: really neex to lookup in ModelLoader::materials or similar
				);
			}

			mdlComp.meshes.reserve(loader.instances.size());
			for (const auto& inst : loader.instances) {
				auto& minfo = meshInfo[inst.meshId];
				mdlComp.meshes.emplace_back(inst.nodeId, minfo.mesh, minfo.mat);
			}

			armComp = std::move(loader.arm);
			if (!loader.animations.empty()) {
				animation = std::move(loader.animations[0]);
			}
		}

		if (!armComp.results.empty()) {
			ubo = engine.getBufferManager().create(armComp.results.size() * sizeof(armComp.results[0]), StorageFlag::DynamicStorage);

			// TODO: work on removing - still needed atm for bone anim
			glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo->get()); // Bind index to ubo
			glUniformBlockBinding(mats[0]->base->getShader()->get(), 0, 1); // Bind uniform block to buffer index
		}
	}

	AnimSystem::~AnimSystem() {
		glDeleteBuffers(1, &cmdbuff);
	}

	void AnimSystem::updateAnim() {
		auto& armComp = world.getComponent<ArmatureComponent>(ent);
		const auto nodeCount = armComp.nodes.size();
		const auto tick = fmodf(clock() / 80.0f, animation.duration);
		armComp.apply(animation, tick);
		if (ubo) { ubo->setData(armComp.results); }
	}

	void AnimSystem::render(const RenderLayer layer) {
		if (layer != RenderLayer::Debug) { return; }

		updateAnim();

		auto vp = glm::ortho<float32>(0, 1920, 0, 1080, -10000, 10000);
		vp = engine.getCamera().getProjection();
		vp *= glm::scale(glm::mat4{1}, glm::vec3{1.0f / pixelsPerMeter});
		//vp *= glm::scale(glm::mat4{1}, glm::vec3{1.0f / 2});

		if (!skinned) {
			// TODO: not a great way to handle uniforms, but this should be resolved when we
			// ^^^^: get more comprehensive instance data support. See: MnETMncr
			auto& [meshes] = world.getComponent<ModelComponent>(ent);
			const auto& arm = world.getComponent<ArmatureComponent>(ent);
			for (int i = 0; i < meshes.size(); ++i) {
				auto& inst = meshes[i];
				const auto& node = arm.nodes[inst.nodeId];
				const auto mvp = vp * node.total;
				inst.mvp = mvp;
			}
		} else {
			auto& [meshes] = world.getComponent<ModelComponent>(ent);
			for (auto& inst : meshes) {
				inst.mvp = vp;
			}
		}
	}
}
