// Engine
#include <Engine/Camera.hpp>
#include <Engine/Gfx/BufferManager.hpp>
#include <Engine/Gfx/Material.hpp>
#include <Engine/Gfx/Mesh2.hpp>
#include <Engine/Gfx/ModelLoader.hpp>


// Game
#include <Game/comps/ArmatureComponent.hpp>
#include <Game/comps/AnimationComponent.hpp>
#include <Game/comps/ModelComponent.hpp>
#include <Game/comps/PhysicsInterpComponent.hpp>
#include <Game/systems/AnimSystem.hpp>


namespace Game {
	AnimSystem::AnimSystem(SystemArg arg) : System{arg} {
		bonesBuff = engine.getBufferManager().create();
		mvpBuff = engine.getBufferManager().create();
		idBuff = engine.getBufferManager().create();

		constexpr char fileName[] = "assets/square_no_arm.fbx";
		//constexpr char fileName[] = "assets/wooble.fbx";
		//constexpr char fileName[] = "assets/char_v3.fbx";

		const auto& modelA = engine.getModelLoader().get(fileName);
		const auto& modelB = engine.getModelLoader().get("assets/wooble.fbx");

		constexpr float inc = 3;
		constexpr int count = 5;
		float32 xOff = count * 0.5f * -inc;

		for (int i = 0; i < count; ++i) {
			const auto& mdlData = i&1 ? modelA : modelB;

			auto ent = world.createEntity();
			auto& mdlComp = world.addComponent<ModelComponent>(ent, mdlData);
			auto& armComp = world.addComponent<ArmatureComponent>(ent);
			auto& physInterpComp = world.addComponent<PhysicsInterpComponent>(ent);
			auto& animComp =  world.addComponent<AnimationComponent>(ent);

			physInterpComp.trans.p.x = xOff;
			xOff += inc;

			armComp = mdlData.arm;
			animComp.anim = mdlData.anims[0];
			const bool skinned = armComp.results.size();

			for (auto& inst : mdlComp.meshes) {
				inst.uboBindings.push_back({
					.buff = mvpBuff,
					.index = 0,
					.offset = 0,
					.size = uint16(-1),
				});

				if (skinned) {
					inst.uboBindings.push_back({
						.buff = bonesBuff,
						.index = 1,
						.offset = 0,
						.size = 0,
					});
				}

				inst.vboBindings.push_back({
					.buff = inst.mesh->vbuff,
					.index = 0,
					.offset = 0,
					.size = (uint16)inst.mesh->vstride,
				});
				inst.vboBindings.push_back({
					.buff = idBuff,
					.index = 1,
					.offset = 0,
					.size = sizeof(uint32),
				});
			}
		}
	}

	AnimSystem::~AnimSystem() {
	}

	void AnimSystem::render(const RenderLayer layer) {
		if (layer != RenderLayer::Debug) { return; }

		const auto& animFilter = world.getFilter<ModelComponent, ArmatureComponent, PhysicsInterpComponent>(); // TODO: cache in system

		auto& cam = engine.getCamera();
		glm::mat4 vpT = cam.getProjection() * cam.getView();


		constexpr static auto align256 = [](const auto v) ENGINE_INLINE -> decltype(v) {
			return (v & ~0xFF); // floor(x / 256) * 256
		};

		bonesBuffTemp.clear();
		mvpBuffTemp.clear();
		idBuffTemp.clear();

		uint64 offset = 0;

		const auto entCount = animFilter.size();
		for (int i=0; auto ent : animFilter) {
			const auto& animComp = world.getComponent<AnimationComponent>(ent);
			auto& armComp = world.getComponent<ArmatureComponent>(ent);
			
			const auto nodeCount = armComp.nodes.size();
			const auto off = CLOCKS_PER_SEC / entCount;
			auto interp = ((clock() + i++*off) % CLOCKS_PER_SEC) / float32(CLOCKS_PER_SEC);
			armComp.apply(*animComp.anim, interp * animComp.anim->duration);
		}

		for (auto ent : animFilter) {
			const auto& arm = world.getComponent<ArmatureComponent>(ent);
			const auto sz = arm.results.size() * sizeof(arm.results[0]);
			const bool skinned = sz;

			if (skinned) {
				// Pad for alignment
				// UBO offsets must be aligned at 256.
				// Some AMD and Intel gpus require less. Most (all?) NVIDIA is 256.
				// If we really care you can query GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT.
				if (auto aligned = align256(offset); aligned != offset) {
					aligned += 256;
					bonesBuffTemp.resize(bonesBuffTemp.size() + (aligned - offset));
					offset = aligned;
				}

				// TODO: only skinned
				// Insert bones
				const auto addr = [](auto it) ENGINE_INLINE { return reinterpret_cast<const byte*>(std::to_address(it)); };
				bonesBuffTemp.insert(bonesBuffTemp.cend(), addr(arm.results.cbegin()), addr(arm.results.cend()));

				// Update bindings
				ENGINE_DEBUG_ASSERT(offset < (1<<16), "Too many bones in armature.");
				ENGINE_DEBUG_ASSERT(sz < (1<<16), "Too many bones in armature.");
			}

			const auto& pos = world.getComponent<PhysicsInterpComponent>(ent).getPosition();
			const auto mT = glm::translate(glm::mat4{1.0f}, glm::vec3{pos.x, pos.y, 0});

			// TODO: really need a way to set a binding point per model as well as per mesh.  kinda feels like we are leaking draw commands at this point.
			auto& mdl = world.getComponent<ModelComponent>(ent);
			for (auto& inst : mdl.meshes) {
				// We could just use baseInstance instead of an id buffer, but that will only work until we get multi draw setup
				inst.baseInstance = static_cast<uint32>(mvpBuffTemp.size());

				if (skinned) { // TODO: only skinned
					inst.uboBindings[1].offset = static_cast<uint16>(offset);
					inst.uboBindings[1].size = static_cast<uint16>(sz);
				}

				idBuffTemp.push_back(inst.baseInstance);
				mvpBuffTemp.push_back(skinned ? vpT * mT : vpT * mT * arm.nodes[inst.nodeId].total);
			}

			if (skinned) {
				offset += sz;
			}
		}

		if (auto sz = bonesBuffTemp.size(); sz > bonesBuffSize) {
			bonesBuffSize = sz;
			bonesBuff->alloc(bonesBuffTemp, Engine::Gfx::StorageFlag::DynamicStorage);
		} else if (sz) {
			bonesBuff->setData(bonesBuffTemp);
		}

		if (auto sz = mvpBuffTemp.size(); sz > mvpBuffSize) {
			mvpBuffSize = sz;
			mvpBuff->alloc(mvpBuffTemp, Engine::Gfx::StorageFlag::DynamicStorage);
		} else if (sz) {
			mvpBuff->setData(mvpBuffTemp);
		}

		if (auto sz = idBuffTemp.size(); sz > idBuffSize) {
			idBuffSize = sz;
			idBuff->alloc(idBuffTemp, Engine::Gfx::StorageFlag::DynamicStorage);
		} else if (sz) {
			idBuff->setData(idBuffTemp);
		}
	}
}
