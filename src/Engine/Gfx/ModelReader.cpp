// Assimp
#include <assimp/postprocess.h>
#include <assimp/scene.h>

// Engine
#include <Engine/ArrayView.hpp>
#include <Engine/Gfx/ModelReader.hpp>


namespace {
	// Convert to column major and glm
	glm::mat4 cvtMat(const aiMatrix4x4& m) {
		return {
			{m.a1, m.b1, m.c1, m.d1},
			{m.a2, m.b2, m.c2, m.d2},
			{m.a3, m.b3, m.c3, m.d3},
			{m.a4, m.b4, m.c4, m.d4},
		};
	};

	ENGINE_INLINE std::string_view view(const aiString& str) {
		return {str.data, str.length};
	}
}

namespace Engine::Gfx {
	ModelReader::ModelReader() {
	}

	void ModelReader::init() {
		clear();

		int numVerts = 0;
		int numFaces = 0;
		int numNodesEst = 0;
		res.skinned = false;

		for(const auto* mesh : ArrayView{scene->mMeshes, scene->mNumMeshes}) {
			numVerts += mesh->mNumVertices;
			numFaces += mesh->mNumFaces;
			numNodesEst += mesh->mNumBones;
			res.skinned = res.skinned || mesh->mNumBones;
		}

		res.verts.resize(numVerts);
		res.indices.resize(numFaces * 3);
		res.meshes.reserve(scene->mNumMeshes);
		res.animations.reserve(scene->mNumAnimations);
		res.instances.reserve(scene->mNumMeshes * 2); // Just a guess, no way to know without walking scene.

		numNodesEst *= 2; // Just a guess, no way to know without walking scene.
		res.arm.reserve(numNodesEst);
		nodeNameToId.reserve(numNodesEst);
	}

	void ModelReader::clear() {
		indexCount = 0;
		vertCount = 0;
		nodeNameToId.clear();

		res.indices.clear();
		res.verts.clear();
		res.meshes.clear();
		res.animations.clear();
		res.instances.clear();
		res.arm.clear();
	}

	bool ModelReader::load(const char* path) {
		scene = im.ReadFile(path, aiProcessPreset_TargetRealtime_Fast);

		if (!scene) {
			ENGINE_WARN("Assimp failed to load model: ", im.GetErrorString());
			return false;
		} else {
			/*ENGINE_LOG("Assimp successfully loaded model: ",
				scene->mNumMeshes, " ",
				scene->mNumAnimations, " ",
				scene->mNumMaterials, " ",
				scene->mNumTextures
			);*/
		}

		init();

		res.materials.resize(scene->mNumMaterials);
		for (unsigned i = 0; i < scene->mNumMaterials; ++i) {
			const aiMaterial* from = scene->mMaterials[i];
			auto& to = res.materials[i];

			for (const auto* prop : ArrayView{from->mProperties, from->mNumProperties}) {
				if (prop->mType != aiPTI_String) { continue; }

				const std::string_view key = {prop->mKey.data, prop->mKey.length};
				const aiString* val = reinterpret_cast<const aiString*>(prop->mData);

				if (key == "?mat.name") {
					to.name = std::string{val->data, val->length};
				} else if (key == "$tex.file") {
					to.path = std::string{val->data, val->length};
				}
			}
		}

		// TODO: there are also other nodes we might want to exclude?
		// Build children. Exclude the root node because its transform is irrelevant (should always be identity)
		//build(scene->mRootNode, -1);
		for (uint i = 0; i < scene->mRootNode->mNumChildren; ++i) {
			build(scene->mRootNode->mChildren[i], -1);
		}

		// TODO: i think we really need to walk the scene, there may be multiple instances of a mesh. Also consider that each instance of a mesh would probably have its own anim time.
		for(const auto* mesh : ArrayView{scene->mMeshes, scene->mNumMeshes}) {
			readMesh(mesh);
		}

		if (res.arm.boneOffsets.size() > 100) { // TODO: make constant or pull from shader or something (or inject into shader? that probably better.)
			ENGINE_WARN("To many bones in model. Clamping. ", path);
			ENGINE_DIE; // TODO: clamp number of bones
		}

		ENGINE_LOG("*** Nodes: ", res.arm.nodes.size(), " / ", res.arm.boneOffsets.size());

		for (const auto* anim : ArrayView{scene->mAnimations, scene->mNumAnimations}) {
			readAnim(anim);
		}

		res.arm.finalize();
		return true;
	}

	void ModelReader::readMesh(const aiMesh* mesh) {
		for (uint i = 0; i < mesh->mNumBones; ++i) {
			const auto& bone = mesh->mBones[i];
			const auto nodeId = getNodeId(bone->mName);

			// We dont need bone data if it doesnt directly effect weights. Only node data.
			if (bone->mNumWeights == 0) { continue; }

			const auto boneId = static_cast<BoneId>(res.arm.boneOffsets.size());
			res.arm.nodes[nodeId].boneId = boneId;

			// TODO: offset matrix will be diff for every mesh that uses this bone, correct? will need multiple stores.
			//			^^^ dont think this is correct? each mesh should have the same offset for bone? test. ^^^
			
			// TODO: multiple meshes might refer to the same bone. Need to handle that
			res.arm.boneOffsets.emplace_back() = cvtMat(bone->mOffsetMatrix);

			for (const auto& weight : ArrayView{bone->mWeights, bone->mNumWeights}) {
				res.verts[weight.mVertexId].addBone(boneId, weight.mWeight);
			}
		}

		++res.materials[mesh->mMaterialIndex].count;
		res.meshes.emplace_back(indexCount, mesh->mNumFaces * 3, mesh->mMaterialIndex);

		const auto baseVertex = vertCount;

		for(const auto& face : ArrayView{mesh->mFaces, mesh->mNumFaces}) {
			ENGINE_ASSERT(face.mNumIndices == 3, "Invalid number of mesh face indices"); // TODO: handle error, dont assert
			ENGINE_DEBUG_ASSERT(indexCount+2 < res.indices.size());
			res.indices[indexCount] = baseVertex + face.mIndices[0];
			res.indices[++indexCount] = baseVertex + face.mIndices[1];
			res.indices[++indexCount] = baseVertex + face.mIndices[2];
			++indexCount;
		}

		ENGINE_DEBUG_ASSERT(indexCount == res.meshes.back().offset + res.meshes.back().count);

		for (uint32 i = 0; i < mesh->mNumVertices; ++i) {
			auto& vert = res.verts[vertCount];

			const auto& pos = mesh->mVertices[i];
			vert.pos = {pos.x, pos.y, pos.z};

			if (const auto& set = mesh->mTextureCoords[0]) {
				ENGINE_ASSERT_WARN(set != nullptr, "Missing UV map for mesh.");
				const auto& uv = mesh->mTextureCoords[0][i];
				static_assert(std::same_as<uint16, std::decay_t<decltype(vert.uv.x)>>, "UVs are currently stored in normalized short format.");
				vert.uv = {
					std::clamp(uv.x * 65535.0f, 0.0f, 65535.0f),
					std::clamp(uv.y * 65535.0f, 0.0f, 65535.0f),
				};
			}

			++vertCount;
		}
	}

	void ModelReader::readAnim(const aiAnimation* anim2) {
		//ENGINE_LOG(
		//	"\n\tName: ", anim2->mName.C_Str(),
		//	"\n\tTicks/s: ", anim2->mTicksPerSecond,
		//	"\n\tDuration: ", anim2->mDuration,
		//	"\n\tChannels:", anim2->mNumChannels,
		//	"\n\tMeshChannels:", anim2->mNumMeshChannels,
		//	""
		//);

		auto& anim = res.animations.emplace_back();
		// TODO: apply mTicksPerSecond to times?
		anim.duration = static_cast<float32>(anim2->mDuration);
		for (const auto* chan : ArrayView{anim2->mChannels, anim2->mNumChannels}) {
			//ENGINE_LOG("\tChannel: ", chan->mNodeName.C_Str(), " ", chan->mNumPositionKeys, " ", chan->mNumRotationKeys, " ", chan->mNumScalingKeys);
			auto& seq = anim.channels.emplace_back();
			seq.nodeId = getNodeId(chan->mNodeName);

			seq.pos.resize(chan->mNumPositionKeys);
			for (auto* cval = chan->mPositionKeys; auto& sval : seq.pos) {
				sval.value.x = cval->mValue.x; sval.value.y = cval->mValue.y; sval.value.z = cval->mValue.z;
				sval.time = static_cast<float32>(cval->mTime);
				++cval;
			}

			seq.scale.resize(chan->mNumScalingKeys);
			for (auto* cval = chan->mScalingKeys; auto& sval : seq.scale) {
				sval.value.x = cval->mValue.x; sval.value.y = cval->mValue.y; sval.value.z = cval->mValue.z;
				sval.time = static_cast<float32>(cval->mTime);
				++cval;
			}

			seq.rot.resize(chan->mNumRotationKeys);
			for (auto* cval = chan->mRotationKeys; auto& sval : seq.rot) {
				sval.value.w = cval->mValue.w; sval.value.x = cval->mValue.x; sval.value.y = cval->mValue.y; sval.value.z = cval->mValue.z;
				sval.time = static_cast<float32>(cval->mTime);
				++cval;
			}
		}

		// Keep everything sorted to more closely resemble access order
		std::sort(anim.channels.begin(), anim.channels.end(), [](const auto& a, const auto& b){
			return a.nodeId < b.nodeId;
		});
	}

	size_t depth = 0; // TODO: rm 
	void ModelReader::build(aiNode* node, NodeId parentId) {
		ENGINE_INFO("BUILD NODE: ", std::string(depth, ' '), node->mName.data, " ", node->mNumMeshes, " @ ", cvtMat(node->mTransformation));
		++depth;
		// Populate nodes
		NodeId nodeId = static_cast<NodeId>(res.arm.nodes.size());
		res.arm.nodes.emplace_back(parentId, -1, cvtMat(node->mTransformation));
		nodeNameToId.emplace(view(node->mName), nodeId);

		#if ENGINE_DEBUG
			res.arm.nodes.back().name = node->mName.C_Str();
		#endif

		// Populate mesh instances
		for (uint i = 0; i < node->mNumMeshes; ++i) {
			res.instances.push_back({
				.meshId = static_cast<MeshId>(node->mMeshes[i]),
				.nodeId = nodeId,
			});
		}

		// Build children
		for (uint i = 0; i < node->mNumChildren; ++i) {
			build(node->mChildren[i], nodeId);
		}
		--depth;
	}

	NodeId ModelReader::getNodeId(const aiString& name) {
		ENGINE_DEBUG_ASSERT(nodeNameToId.contains(view(name)));
		return nodeNameToId[view(name)];
	};
}
