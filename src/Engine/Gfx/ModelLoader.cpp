// Engine
#include <Engine/Gfx/ModelLoader.hpp>
#include <Engine/Gfx/VertexAttributeLayout.hpp>
#include <Engine/Gfx/ResourceContext.hpp>


namespace Engine::Gfx {
	void ModelLoader::load(const std::string_view path, ModelData& data) {
		using namespace Engine::Gfx;

		if (!reader.load(std::string{path}.c_str())) {
			ENGINE_WARN("Failed to load model: ", path);
			return;
		}

		const auto& mdl = reader.results();
		const bool skinned = mdl.skinned;

		ENGINE_INFO("**** Loaded Model: ", mdl.verts.size(), " ", mdl.indices.size(), " ", mdl.instances.size(), " ", skinned);

		constexpr VertexAttributeDesc attribs[] = {
			{ +VertexInput::Position, 3, NumberType::Float32, VertexAttribTarget::Float, false, offsetof(Vertex, pos), 0, 0},
			{ +VertexInput::TexCoord, 2, NumberType::UInt16, VertexAttribTarget::Float, true, offsetof(Vertex, uv), 0, 0},
			{ +VertexInput::BoneIndices, 4, NumberType::UInt8, VertexAttribTarget::Int, false, offsetof(Vertex, bones), 0, 0},
			{ +VertexInput::BoneWeights, 4, NumberType::Float32, VertexAttribTarget::Float, false, offsetof(Vertex, weights), 0, 0},
			{ +VertexInput::DrawId, 1, NumberType::UInt32, VertexAttribTarget::Int, false, 0, 1, 1},
		};

		auto layout = rctx.vertexLayoutLoader.get(attribs);

		std::vector<MaterialInstanceRefWeak> mats;
		mats.reserve(mdl.materials.size());
		
		for (const auto& from : mdl.materials) {
			if (from.count == 0) {
				ENGINE_WARN("Unused material: ", from.name);
				continue;
			}

			ENGINE_LOG("Load Material(", from.count, "): ", from.path);
			mats.emplace_back(rctx.materialInstanceLoader.get({
				.path = from.path,
				.shdr = skinned ? "shaders/mesh" : "shaders/mesh_static",
			}));
		}

		const auto vbo = rctx.bufferManager.create(mdl.verts);
		const auto ebo = rctx.bufferManager.create(mdl.indices);

		struct MeshInfo {
			MeshRef mesh;
			MaterialInstanceRef mat;
		};
		std::vector<MeshInfo> meshInfo;
		meshInfo.reserve(mdl.meshes.size());

		for (auto& m : mdl.meshes) {
			meshInfo.emplace_back(
				rctx.meshManager.create(
					layout,
					vbo, static_cast<uint32>(sizeof(Vertex)),
					ebo, m.offset, m.count
				),
				MaterialInstanceRef(mats[m.material])
			);
		}

		data.meshes.reserve(mdl.instances.size());

		for (const auto& inst : mdl.instances) {
			auto& minfo = meshInfo[inst.meshId];
			data.meshes.emplace_back(minfo.mesh, minfo.mat, inst.nodeId);
		}

		data.arm = mdl.arm;
		data.anims.reserve(mdl.animations.size());
		for (const auto& anim : mdl.animations) {
			data.anims.emplace_back(rctx.animManager.create(anim));
		}

	}
}
