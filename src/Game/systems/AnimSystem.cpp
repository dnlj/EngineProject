// Engine
#include <Engine/Camera.hpp>
#include <Engine/Gfx/BufferManager.hpp>
#include <Engine/Gfx/ModelLoader.hpp>
#include <Engine/Gfx/Mesh2.hpp>
#include <Engine/Gfx/Material.hpp>


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
			//constexpr char fileName[] = "assets/testing.fbx";
			//constexpr char fileName[] = "assets/tri_test3.fbx";
			//constexpr char fileName[] = "assets/tri_test2.fbx";///////////////
			//constexpr char fileName[] = "assets/tri_test2_3.fbx";
			//constexpr char fileName[] = "assets/test.fbx";
			constexpr char fileName[] = "assets/char_v2.fbx";//////////////////////////////
			//constexpr char fileName[] = "assets/char6.fbx";///////////////
			//constexpr char fileName[] = "assets/char.glb";
			//constexpr char fileName[] = "assets/char.dae";

			const auto& data = engine.getModelLoader().get(fileName);
			mdlComp.meshes = data.meshes;
			armComp = data.arm;
			animation = data.anims[0];
			skinned = !armComp.boneOffsets.empty(); // TODO: handle better
		}

		if (!armComp.results.empty()) {
			ubo = engine.getBufferManager().create(armComp.results.size() * sizeof(armComp.results[0]), StorageFlag::DynamicStorage);

			// TODO: work on removing - still needed atm for bone anim
			glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo->get()); // Bind index to ubo

			// TODO: awful way of doing this.
			//const auto shader = mdlComp.meshes[0].material->base->getShader()->get();
			//glUniformBlockBinding(shader, 0, 1); // Bind uniform block to buffer index
		}
	}

	AnimSystem::~AnimSystem() {
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
		//vp *= glm::scale(glm::mat4{1}, glm::vec3{1.0f / 0.2f});

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
