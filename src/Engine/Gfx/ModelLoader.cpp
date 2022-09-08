// Engine
#include <Engine/Gfx/ModelLoader.hpp>
#include <Engine/Gfx/VertexAttributeLayout.hpp>
#include <Engine/Gfx/ResourceContext.hpp>


namespace Engine::Gfx {
	void ModelLoader::load(const std::string_view path, ModelData& data) {
		using namespace Engine::Gfx;

		reader.load(std::string{path}.c_str());

		bool skinned = !reader.arm.boneOffsets.empty(); // TODO: better way to handle this
		ENGINE_INFO("**** Loaded Model: ", reader.verts.size(), " ", reader.indices.size(), " ", reader.instances.size(), " ", skinned);

		VertexAttributeDesc attribs[] = {
			{ VertexInput::Position, 3, NumberType::Float32, offsetof(Vertex, pos), false },
			{ VertexInput::TexCoord, 2, NumberType::Float32, offsetof(Vertex, uv), false }, // TODO: use normalized short here like we do in DrawBuilder
			{ VertexInput::BoneIndices, 4, NumberType::UInt8, offsetof(Vertex, bones), false },
			{ VertexInput::BoneWeights, 4, NumberType::Float32, offsetof(Vertex, weights), false },
		};

		auto layout = rctx.vertexLayoutLoader.get(attribs);

		{ // TODO: load from model
			const auto shader = rctx.shaderLoader.get(skinned ? "shaders/mesh" : "shaders/mesh_static");
			auto matBase = rctx.materialManager.create(shader);
			//auto tex = rctx.textureLoader.get2D("assets/gui_1.bmp");
			auto tex1 = rctx.textureLoader.get2D("assets/tri_test3_uv1.png");
			//auto tex2 = rctx.textureLoader.get2D("assets/tri_test3_uv2.png");
			auto tex2 = rctx.textureLoader.get2D("assets/char_uv.png");
			//ENGINE_LOG("Texture = ", tex->tex.get());

			mats[0] = rctx.materialInstanceManager.create(matBase);
			mats[0]->set("color", glm::vec4{1,1,0.5,1});
			mats[0]->set("tex", tex2);

			mats[1] = rctx.materialInstanceManager.create(matBase);
			mats[1]->set("color", glm::vec4{1,0.5,1,1});
			mats[1]->set("tex", tex1);

			mats[2] = rctx.materialInstanceManager.create(matBase);
			mats[2]->set("color", glm::vec4{0.5,1,1,1});
			mats[2]->set("tex", tex1);
		}

		const auto vbo = rctx.bufferManager.create(reader.verts);
		const auto ebo = rctx.bufferManager.create(reader.indices);

		struct MeshInfo {
			MeshRef mesh;
			MaterialInstanceRef mat;
		};
		std::vector<MeshInfo> meshInfo;
		meshInfo.reserve(reader.meshes.size());

		for (auto& m : reader.meshes) {
			meshInfo.emplace_back(
				rctx.meshManager.create(
					layout,
					vbo, static_cast<uint32>(sizeof(Vertex)),
					ebo, m.offset, m.count
				),
				mats[m.material] // TODO: really neex to lookup in ModelLoader::materials or similar
			);
		}

		data.meshes.reserve(reader.instances.size());

		for (const auto& inst : reader.instances) {
			auto& minfo = meshInfo[inst.meshId];
			data.meshes.emplace_back(inst.nodeId, minfo.mesh, minfo.mat);
		}

		data.arm = std::move(reader.arm);
		data.anims = std::move(reader.animations);
	}
}
