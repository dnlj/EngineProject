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

			mdlComp.meshes.reserve(data.meshes.size());
			for (const auto& inst : data.meshes) {
				mdlComp.meshes.push_back({
					.mesh = inst.mesh,
					.mat = inst.mat,
					.nodeId = inst.nodeId,
				});
			}

			armComp = data.arm;
			animation = data.anims[0];
			skinned = !armComp.boneOffsets.empty(); // TODO: handle better
		}

		if (!armComp.results.empty()) {
			const auto size = armComp.results.size() * sizeof(armComp.results[0]);
			ubo = engine.getBufferManager().create(size, StorageFlag::DynamicStorage);

			for (auto& inst : mdlComp.meshes) {
				ENGINE_DEBUG_ASSERT(size <= (1<<16));
				inst.bindings.push_back({
					.buff = ubo,
					.index = 0,
					.offset = 0,
					.size = static_cast<uint16>(size),
				});
			}
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

		auto& modelComp = world.getComponent<ModelComponent>(ent);
		if (!skinned) {
			// TODO: not a great way to handle uniforms, but this should be resolved when we
			// ^^^^: get more comprehensive instance data support. See: MnETMncr
			const auto& arm = world.getComponent<ArmatureComponent>(ent);
			for (auto& inst : modelComp.meshes) {
				const auto& node = arm.nodes[inst.nodeId];
				const auto mvp = vp * node.total;
				inst.mvp = mvp;
			}
		} else {
			//auto& meshes = world.getComponent<ModelComponent>(ent).meshes;
			for (auto& inst : modelComp.meshes) {
				inst.mvp = vp;
			}
			// TODO: just modelComp.mvp = vp;
		}
	}
}
