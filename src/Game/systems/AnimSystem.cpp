// Engine
#include <Engine/Camera.hpp>
#include <Engine/Gfx/MaterialManager.hpp>
#include <Engine/Gfx/MaterialInstanceManager.hpp>
#include <Engine/Gfx/Mesh.hpp>
#include <Engine/Gfx/VertexAttributeLayout.hpp>
#include <Engine/Gfx/VertexLayoutLoader.hpp>
#include <Engine/Gfx/ShaderLoader.hpp>
#include <Engine/Gfx/BufferManager.hpp>
#include <Engine/Gfx/MeshManager.hpp>

// Game
#include <Game/systems/AnimSystem.hpp>
#include <Game/comps/ModelComponent.hpp>

namespace {
	typedef struct {
        uint32_t  count;
        uint32_t  instanceCount;
        uint32_t  firstIndex;
         int32_t  baseVertex;
        uint32_t  baseInstance;
    } DrawElementsIndirectCommand;
	static_assert(sizeof(DrawElementsIndirectCommand) == 4 * 5, "glMultiDrawElementsIndirect requires that the indirect structure is tightly packed if using a stride of zero.");
}


namespace Game {
	AnimSystem::AnimSystem(SystemArg arg) : System{arg} {
		using namespace Engine::Gfx;

		ent = world.createEntity();

		{
			VertexAttributeDesc attribs[] = {
				{ VertexInput::Position, 3, NumberType::Float32, offsetof(Vertex, pos), false },
				{ VertexInput::BoneIndices, 4, NumberType::UInt8, offsetof(Vertex, bones), false },
				{ VertexInput::BoneWeights, 4, NumberType::Float32, offsetof(Vertex, weights), false },
			};

			auto layout = engine.getVertexLayoutLoader().get(attribs);

			// TODO: we probably also want a ModelLoader/Manager to cache this stuff so we dont load the same thing multiple times
			ModelLoader loader;
			model.skinned = !loader.arm.bones.empty();

			{
				const auto shader = engine.getShaderLoader().get(model.skinned ? "shaders/mesh" : "shaders/mesh_static");
				const auto matBase = engine.getMaterialManager().create(shader);

				// TODO: load from model
				mats[0] = engine.getMaterialInstanceManager().create(matBase);
				mats[0]->params.set(123, glm::vec4{1,1,0.5,1});

				mats[1] = engine.getMaterialInstanceManager().create(matBase);
				mats[1]->params.set(123, glm::vec4{1,0.5,1,1});

				mats[2] = engine.getMaterialInstanceManager().create(matBase);
				mats[2]->params.set(123, glm::vec4{0.5,1,1,1});
			}

			ENGINE_INFO("**** Loaded Model: ", loader.verts.size(), " ", loader.indices.size(), " ", loader.instances.size());

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

			auto& mdlComp = world.addComponent<ModelComponent>(ent);
			mdlComp.meshes.reserve(loader.instances.size());
			for (const auto& inst : loader.instances) {
				auto& minfo = meshInfo[inst.meshId];
				mdlComp.meshes.emplace_back(inst.nodeId, minfo.mesh, minfo.mat);
			}

			// TODO: really arm.bones is the same for all instances of a mesh, its just the offset matrix. Would it make sense to ref-ify armatures?
			model.arm = std::move(loader.arm);
			if (!loader.animations.empty()) {
				animation = std::move(loader.animations[0]);
			}
		}

		model.bones.resize(model.arm.bones.size());

		if (!model.bones.empty()) {
			ubo = engine.getBufferManager().create(model.bones.size() * sizeof(model.bones[0]), StorageFlag::DynamicStorage);

			// TODO: work on removing - still needed atm for bone anim
			glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo->get()); // Bind index to ubo
			glUniformBlockBinding(mats[0]->base->getShader()->get(), 0, 1); // Bind uniform block to buffer index
		}
	}

	AnimSystem::~AnimSystem() {
		glDeleteBuffers(1, &cmdbuff);
	}

	void AnimSystem::updateAnim() {
		const auto nodeCount = model.arm.nodes.size();
		const auto tick = fmodf(clock() / 80.0f, animation.duration);

		for (const auto& seq : animation.channels) {
			const auto& interp = seq.interp(tick);
			model.arm.nodes[seq.nodeId].trans = glm::scale(glm::translate(glm::mat4{1.0f}, interp.pos) * glm::mat4_cast(interp.rot), interp.scale);
		}

		for (Engine::Gfx::NodeId ni = 0; ni < nodeCount; ++ni) {
			auto& node = model.arm.nodes[ni];
			if (node.parentId >= 0) {
				node.total = model.arm.nodes[node.parentId].total * node.trans;
			} else {
				node.total = node.trans;
			}

			if (node.boneId >= 0) {
				model.bones[node.boneId] = node.total * model.arm.bones[node.boneId].offset;
			}
		}

		if (ubo) { ubo->setData(model.bones); }
	}

	void AnimSystem::render(const RenderLayer layer) {
		if (layer != RenderLayer::Debug) { return; }

		updateAnim();

		auto vp = glm::ortho<float32>(0, 1920, 0, 1080, -10000, 10000);
		vp = engine.getCamera().getProjection();
		vp *= glm::scale(glm::mat4{1}, glm::vec3{1.0f / pixelsPerMeter});
		//vp *= glm::scale(glm::mat4{1}, glm::vec3{1.0f / 2});

		if (!model.skinned) {
			// TODO: Awful way to handle this. Very fragile.
			// ^^^^: Not sure what we ment by this? I assume because we use to index into the model.instances with the same index as the meshes.
			// ^^^^: Which we just happen to know align but could be different?
			// ^^^^: Investigate more then remove if there isnt anything obviously wrong here.
			auto& [meshes] = world.getComponent<ModelComponent>(ent);
			for (int i = 0; i < meshes.size(); ++i) {
				auto& inst = meshes[i];
				const auto& node = model.arm.nodes[inst.nodeId];
				const auto mvp = vp * node.total;
				inst.mvp = mvp;
			}
		} else {
			auto& [meshes] = world.getComponent<ModelComponent>(ent);
			for (auto& inst : meshes) {
				inst.mvp = vp;
			}
			/*
			glUseProgram(shaderSkinned->get());
			glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);

			static std::vector<DrawElementsIndirectCommand> commands; // TODO: rm temp

			if (cmdbuff == 0) {
				for (const auto& inst : model.instances) {
					const auto& mesh = inst.mesh;
					commands.push_back({
						.count = mesh->ecount,
						.instanceCount = 1,
						.firstIndex = mesh->eoffset,
						.baseVertex = 0,
						.baseInstance = 0,
					});
				}

				glCreateBuffers(1, &cmdbuff);
				glNamedBufferStorage(cmdbuff, commands.size() * sizeof(commands[0]), nullptr, GL_DYNAMIC_STORAGE_BIT);
				glNamedBufferSubData(cmdbuff, 0, commands.size() * sizeof(commands[0]), std::data(commands));
			}

			glBindVertexArray(layout->vao);
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, cmdbuff);
			glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, (GLsizei)std::size(commands), 0);*/
		}
	}
}
