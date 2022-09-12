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

		bonesBuff = engine.getBufferManager().create();
	}

	AnimSystem::~AnimSystem() {
	}

	void AnimSystem::updateAnim() {
		auto& armComp = world.getComponent<ArmatureComponent>(ent);
		const auto nodeCount = armComp.nodes.size();
		const auto tick = fmodf(clock() / 80.0f, animation.duration);
		armComp.apply(animation, tick);
	}

	void AnimSystem::render(const RenderLayer layer) {
		using namespace Engine::Gfx;

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

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		const auto& armFilter = world.getFilter<ModelComponent, ArmatureComponent>(); // TODO: cache in system

		constexpr auto align256 = [](const auto v) ENGINE_INLINE -> decltype(v) {
			return (v & ~0xff); // floor(x / 256) * 256
		};

		{
			bonesBuffTemp.clear();
			uint64 offset = 0;

			// TODO: we really need to get multiple models working so we can test this properly
			for (auto ent : armFilter) {
				// Pad for alignment
				// UBO offsets must be aligned at 256.
				// Some AMD and Intel gpus require less. Most (all?) NVIDIA is 256.
				// If we really care you can query GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT.
				if (auto aligned = align256(offset); aligned != offset) {
					aligned += 256;
					bonesBuffTemp.resize(bonesBuffTemp.size() + (aligned - offset));
					offset = aligned;
				}

				// Insert new elements
				const auto& arm = world.getComponent<ArmatureComponent>(ent);
				ENGINE_LOG("Update: ", ent, " ", arm.results.size());
				const auto addr = [](auto it) ENGINE_INLINE { return reinterpret_cast<const byte*>(std::to_address(it)); };
				bonesBuffTemp.insert(bonesBuffTemp.cend(), addr(arm.results.cbegin()), addr(arm.results.cend()));

				// Update bindings
				auto& mdl = world.getComponent<ModelComponent>(ent);
				const auto sz = arm.results.size() * sizeof(arm.results[0]);
				ENGINE_DEBUG_ASSERT(offset < (1<<16), "Too many bones in armature.");
				ENGINE_DEBUG_ASSERT(sz < (1<<16), "Too many bones in armature.");

				// TODO: really need a way to set a binding point per model as well as per mesh.  kinda feels like we are leaking draw commands at this point.
				// TODO: impl this, just copy opasted from above. change vals
				for (auto& inst : mdl.meshes) {
					inst.bindings.clear(); // TODO: better way to handle this. we really dont want to clear this. other systems might set buffer bindings.
					inst.bindings.push_back({
						.buff = bonesBuff,
						.index = 0,
						.offset = static_cast<uint16>(offset),
						.size = static_cast<uint16>(sz),
					});
				}

				offset += sz;
			}

			if (offset) {
				if (auto sz = bonesBuffTemp.size(); sz > bonesBuffSize) {
					ENGINE_INFO(" ** RESIZE BONES BUFFER ** ", sz);
					bonesBuffSize = sz;
					bonesBuff->alloc(bonesBuffTemp, StorageFlag::DynamicStorage);
				} else {
					bonesBuff->setData(bonesBuffTemp);
				}
			}
		}
	}
}
