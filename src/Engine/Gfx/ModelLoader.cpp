// Assimp
#include <assimp/postprocess.h>
#include <assimp/scene.h>

// Engine
#include <Engine/ArrayView.hpp>
#include <Engine/Gfx/ModelLoader.hpp>


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
	ModelLoader::ModelLoader() {

		// TODO: try "nested planes" with each one having a different material
		// TODO: try nested armature

		//constexpr char fileName[] = "assets/testing.fbx";
		//constexpr char fileName[] = "assets/tri_test2.fbx";
		//constexpr char fileName[] = "assets/test.fbx";
		constexpr char fileName[] = "assets/char6.fbx";
		//constexpr char fileName[] = "assets/char.glb";
		//constexpr char fileName[] = "assets/char.dae";

		load(fileName);
	}

	void ModelLoader::init() {
		clear();

		int numVerts = 0;
		int numFaces = 0;
		int numNodesEst = 0;
		skinned = false;

		for(const auto* mesh : Engine::ArrayView{scene->mMeshes, scene->mNumMeshes}) {
			numVerts += mesh->mNumVertices;
			numFaces += mesh->mNumFaces;
			numNodesEst += mesh->mNumBones;
			skinned = skinned || mesh->mNumBones;
		}

		// TODO: separate rendering paths for skinned/static meshes?
		ENGINE_WARN("TODO: hasBones ", skinned, " ", scene->mNumAnimations);

		// TODO: glMapBuffer w/o temporary buffer instead? would be good to test how that effects load times.
		verts.resize(numVerts);
		indices.resize(numFaces * 3);
		meshes.reserve(scene->mNumMeshes);
		animations.reserve(scene->mNumAnimations);
		instances.reserve(scene->mNumMeshes * 2); // Just a guess, no way to know without walking scene.

		numNodesEst *= 2;  // Just a guess, no way to know without walking scene.
		arm.nodes.reserve(numNodesEst);
		arm.bones.reserve(numNodesEst);
		nodeNameToId.reserve(numNodesEst);
	}

	void ModelLoader::clear() {
		indexCount = 0;
		vertCount = 0;
		indices.clear();
		verts.clear();
		nodeNameToId.clear();
		meshes.clear();
		animations.clear();
		instances.clear();
		arm.clear();
	}

	void ModelLoader::load(const char* path) {
		scene = im.ReadFile(path, aiProcessPreset_TargetRealtime_Fast);

		if (!scene) {
			ENGINE_WARN("Assimp failed to load model: ", im.GetErrorString());
			return;
		} else {
			/*ENGINE_LOG("Assimp successfully loaded model: ",
				scene->mNumMeshes, " ",
				scene->mNumAnimations, " ",
				scene->mNumMaterials, " ",
				scene->mNumTextures
			);*/
		}

		init();

		// TODO: there are also other nodes we might want to exclude?
		// Build children. Exclude the root node because its transform is irrelevant (should always be identity)
		//build(scene->mRootNode, -1);
		for (uint i = 0; i < scene->mRootNode->mNumChildren; ++i) {
			build(scene->mRootNode->mChildren[i], -1);
		}

		// TODO: test mesh instancing
		// TODO: i think we really need to walk the scene, there may be multiple instances of a mesh. Also consider that each instance of a mesh would probably have its own anim time.
		for(const auto* mesh : Engine::ArrayView{scene->mMeshes, scene->mNumMeshes}) {
			readMesh(mesh);
		}

		if (arm.bones.size() > 100) { // TODO: make constant or pull from shader or something (or inject into shader? that probably better.)
			ENGINE_WARN("To many bones in model. Clamping. ", path);
			ENGINE_DIE; // TODO: clamp number of bones
		}

		ENGINE_LOG("*** Nodes: ", arm.nodes.size(), " / ", arm.bones.size());

		for (const auto* anim : Engine::ArrayView{scene->mAnimations, scene->mNumAnimations}) {
			readAnim(anim);
		}

		arm.nodes.shrink_to_fit();
		arm.bones.shrink_to_fit();
		instances.shrink_to_fit();
	}

	void ModelLoader::readMesh(const aiMesh* mesh) {
		for (uint i = 0; i < mesh->mNumBones; ++i) {
			const auto& bone = mesh->mBones[i];
			const auto nodeId = getNodeId(bone->mName);

			// We dont need bone data if it doesnt directly effect weights. Only node data.
			if (bone->mNumWeights == 0) { continue; }

			const auto boneId = static_cast<BoneId>(arm.bones.size());
			arm.nodes[nodeId].boneId = boneId;

			// TODO: offset matrix will be diff for every mesh that uses this bone, correct? will need multiple stores.
			//			^^^ dont think this is correct? each mesh should have the same offset for bone? test. ^^^
			
			// TODO: multiple meshes might refer to the same bone. Need to handle that
			arm.bones.emplace_back().offset = cvtMat(bone->mOffsetMatrix);

			for (const auto& weight : Engine::ArrayView{bone->mWeights, bone->mNumWeights}) {
				verts[weight.mVertexId].addBone(boneId, weight.mWeight);
			}
		}

		const auto& range = meshes.emplace_back(indexCount, mesh->mNumFaces * 3, mesh->mMaterialIndex);
		const auto baseVertex = vertCount;

		for(const auto& face : Engine::ArrayView{mesh->mFaces, mesh->mNumFaces}) {
			ENGINE_ASSERT(face.mNumIndices == 3, "Invalid number of mesh face indices"); // TODO: handle error, dont assert
			ENGINE_DEBUG_ASSERT(indexCount+2 < indices.size());
			indices[indexCount] = baseVertex + face.mIndices[0];
			indices[++indexCount] = baseVertex + face.mIndices[1];
			indices[++indexCount] = baseVertex + face.mIndices[2];
			++indexCount;
		}

		ENGINE_DEBUG_ASSERT(indexCount == range.offset + range.count);

		for (const auto& v : Engine::ArrayView{mesh->mVertices, mesh->mNumVertices}) {
			verts[vertCount].pos = {v.x, v.y, v.z};
			++vertCount;
		}
	}

	void ModelLoader::readAnim(const aiAnimation* anim2) {
		ENGINE_LOG(
			"\n\tName: ", anim2->mName.C_Str(),
			"\n\tTicks/s: ", anim2->mTicksPerSecond,
			"\n\tDuration: ", anim2->mDuration,
			"\n\tChannels:", anim2->mNumChannels,
			"\n\tMeshChannels:", anim2->mNumMeshChannels,
			""
		);

		auto& anim = animations.emplace_back();
		// TODO: apply mTicksPerSecond to times?
		anim.duration = static_cast<float32>(anim2->mDuration);
		for (const auto* chan : Engine::ArrayView{anim2->mChannels, anim2->mNumChannels}) {
			ENGINE_LOG("\tChannel: ", chan->mNodeName.C_Str(), " ", chan->mNumPositionKeys, " ", chan->mNumRotationKeys, " ", chan->mNumScalingKeys);
			auto& seq = anim.channels.emplace_back();
			seq.nodeId = getNodeId(chan->mNodeName);
			ENGINE_LOG("ANIM: ", seq.nodeId);

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
	void ModelLoader::build(aiNode* node, NodeId parentId) {
		ENGINE_INFO("BUILD NODE: ", std::string(depth, ' '), node->mName.data, " ", node->mNumMeshes, " @ ", cvtMat(node->mTransformation));
		++depth;
		// Populate nodes
		NodeId nodeId = static_cast<NodeId>(arm.nodes.size());
		arm.nodes.emplace_back(parentId, -1, cvtMat(node->mTransformation));
		nodeNameToId.emplace(view(node->mName), nodeId);

		#if ENGINE_DEBUG
			arm.nodes.back().name = node->mName.C_Str();
		#endif

		// Populate mesh instances
		for (uint i = 0; i < node->mNumMeshes; ++i) {
			instances.push_back({
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

	NodeId ModelLoader::getNodeId(const aiString& name) {
		ENGINE_DEBUG_ASSERT(nodeNameToId.contains(view(name)));
		return nodeNameToId[view(name)];
	};
}