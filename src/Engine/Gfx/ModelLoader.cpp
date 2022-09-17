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
			{ +VertexInput::Position, 3, NumberType::Float32, offsetof(Vertex, pos), false, 0, 0},
			{ +VertexInput::TexCoord, 2, NumberType::Float32, offsetof(Vertex, uv), false, 0, 0}, // TODO: use normalized short here like we do in DrawBuilder
			{ +VertexInput::BoneIndices, 4, NumberType::UInt8, offsetof(Vertex, bones), false, 0, 0},
			{ +VertexInput::BoneWeights, 4, NumberType::Float32, offsetof(Vertex, weights), false, 0, 0},
			{ +VertexInput::DrawId, 1, NumberType::UInt32, 0, false, 1, 1},
		};

		auto layout = rctx.vertexLayoutLoader.get(attribs);

		std::vector<MaterialInstanceRef> mats;
		mats.reserve(reader.materials.size());
		
		const auto shader = rctx.shaderLoader.get(skinned ? "shaders/mesh" : "shaders/mesh_static");
		const auto matBase = rctx.materialLoader.get(shader);
		for (const auto& from : reader.materials) {
			if (from.count == 0) { continue; }
			ENGINE_LOG("Load Material(", from.count, "): ", from.path);
			// TODO: Really need an instance loader, shouldnt create a new one for each model. See: qZnumyMN
			auto& to = mats.emplace_back(rctx.materialInstanceManager.create(matBase));
			auto tex = rctx.textureLoader.get2D("assets/" + from.path); // TODO: better path handling. See: kUYZw2N2
			to->set("color", glm::vec4{1,1,0.5,1});
			to->set("tex", tex);
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
				mats[m.material]
			);
		}

		data.meshes.reserve(reader.instances.size());

		for (const auto& inst : reader.instances) {
			auto& minfo = meshInfo[inst.meshId];
			data.meshes.emplace_back(minfo.mesh, minfo.mat, inst.nodeId);
		}

		data.arm = std::move(reader.arm);
		data.anims = std::move(reader.animations);
	}
}
