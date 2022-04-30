// Assimp
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

// Engine
#include <Engine/Gfx/Mesh.hpp>
#include <Engine/ArrayView.hpp>
#include <Engine/Zip.hpp>

// Game
#include <Game/systems/AnimSystem.hpp>

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
}

namespace Game {
	ModelLoader::ModelLoader() {
		constexpr char fileName[] = "assets/char6.fbx";
		//constexpr char fileName[] = "assets/char.glb";
		//constexpr char fileName[] = "assets/char.dae";

		Assimp::Importer im;
		scene = im.ReadFile(fileName, aiProcessPreset_TargetRealtime_Fast);

		if (!scene) {
			ENGINE_WARN("Assimp failed to load model: ", im.GetErrorString());
			return;
		} else {
			ENGINE_LOG("Assimp successfully loaded model: ",
				scene->mNumMeshes, " ",
				scene->mNumAnimations, " ",
				scene->mNumMaterials, " ",
				scene->mNumTextures
			);
		}

		// TODO: i think we really need to walk the scene, there may be multiple instances of a mesh. Also consider that each instance of a mesh would probably have its own anim time.
		ENGINE_ASSERT(scene->mNumMeshes == 1, "Invalid number of meshes in file"); // TODO: handle error, dont assert
		for(const auto* mesh : Engine::ArrayView{scene->mMeshes, scene->mNumMeshes}) {
			readMesh(mesh);
		}

		if (arm.bones.size() > 100) { // TODO: make constant or pull from shader or something (or inject into shader? that probably better.)
			ENGINE_WARN("To many bones in model. Clamping. ", fileName);
			ENGINE_DIE; // TODO: clamp number of bones
		}

		arm.nodes.shrink_to_fit();
		arm.bones.shrink_to_fit();

		ENGINE_LOG("*** Nodes: ", arm.nodes.size(), " / ", arm.bones.size());

		ENGINE_DEBUG_ASSERT(scene->mNumAnimations == 1); // TODO: support multiple animations
		for (const auto* anim : Engine::ArrayView{scene->mAnimations, scene->mNumAnimations}) {
			readAnim(anim);
		}
	}

	void ModelLoader::readMesh(const aiMesh* mesh) {
		{
			// TODO: do we want ot actually clear anymore? do we pack all messages or seperate vbo?
			indices.clear();
			verts.clear();
			verts.resize(mesh->mNumVertices);
			indices.resize(mesh->mNumFaces * 3);

			const auto estNodes = arm.nodes.size() + mesh->mNumBones * 2; // Just a guess, no way to get true number without walking `scene->mRootNode`
			arm.nodes.reserve(estNodes);
			arm.bones.reserve(estNodes);
			nodeToIndex.reserve(estNodes);
		}
			
		for (uint i = 0; i < mesh->mNumBones; ++i) {
			const auto& bone = mesh->mBones[i];
			const auto nodeId = getNodeId(bone->mName);

			// We dont need bone data if it doesnt directly effect weights. Only node data.
			if (bone->mNumWeights == 0) { continue; }

			const auto boneId = static_cast<BoneId>(arm.bones.size());
			arm.nodes[nodeId].boneId = boneId;
			arm.bones.emplace_back().offset = cvtMat(bone->mOffsetMatrix);

			for (const auto& weight : Engine::ArrayView{bone->mWeights, bone->mNumWeights}) {
				verts[weight.mVertexId].addBone(boneId, weight.mWeight);
			}
		}

		for(uint i = -1; const auto& face : Engine::ArrayView{mesh->mFaces, mesh->mNumFaces}) {
			ENGINE_ASSERT(face.mNumIndices == 3, "Invalid number of mesh face indices"); // TODO: handle error, dont assert
			ENGINE_DEBUG_ASSERT(i+2 < indices.size());
			indices[++i] = face.mIndices[0];
			indices[++i] = face.mIndices[1];
			indices[++i] = face.mIndices[2];
		}

		for (uint i = 0; i < mesh->mNumVertices; ++i) {
			const auto& v =  mesh->mVertices[i];
			verts[i].pos = {v.x, v.y, v.z};
		}
	}

	void ModelLoader::readAnim(const aiAnimation* anim) {
		ENGINE_LOG(
			"\n\tName: ", anim->mName.C_Str(),
			"\n\tTicks/s: ", anim->mTicksPerSecond,
			"\n\tDuration: ", anim->mDuration,
			"\n\tChannels:", anim->mNumChannels,
			"\n\tMeshChannels:", anim->mNumMeshChannels,
			""
		);

		// TODO: apply mTicksPerSecond to times?
		animation.duration = static_cast<float32>(anim->mDuration);
		for (const auto* chan : Engine::ArrayView{anim->mChannels, anim->mNumChannels}) {
			ENGINE_LOG("\tChannel: ", chan->mNodeName.C_Str(), chan->mNumPositionKeys, " ", chan->mNumRotationKeys, " ", chan->mNumScalingKeys);
			const auto name = std::string_view{chan->mNodeName.data, chan->mNodeName.length};
			auto& seq = animation.channels.emplace_back();
			seq.nodeId = nodeToIndex[name];
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
		std::sort(animation.channels.begin(), animation.channels.end(), [](const auto& a, const auto& b){
			return a.nodeId < b.nodeId;
		});
	}

	NodeId ModelLoader::getNodeId(const aiString& name, const aiNode* node) {
		auto found = nodeToIndex.find(std::string_view{name.data, name.length});
		if (found == nodeToIndex.end()) {
			if (!node) {
				node = scene->mRootNode->FindNode(name);
			}

			NodeId parent = -1;
			// TODO: need to stop at mesh or mesh parent node. I think bones dont NEED to be in one of these places but in practice they will. If they arent we can say the model is ill formed and should be fixed on the model side, not in code
			if (node->mParent && node->mParent != scene->mRootNode) {
				parent = getNodeId(node->mParent->mName, node->mParent);
			}

			found = nodeToIndex.emplace(std::string_view{name.data, name.length}, static_cast<NodeId>(arm.nodes.size())).first;
			arm.nodes.emplace_back(parent, -1, cvtMat(node->mTransformation));

			#if ENGINE_DEBUG
				arm.nodes.back().name = found->first;
			#endif

			ENGINE_LOG("Build Node: ", found->second, " - ", node->mName.C_Str(), " - ", cvtMat(node->mTransformation));
		}

		return found->second;
	};
}

namespace Game {
	AnimSystem::AnimSystem(SystemArg arg) : System{arg} {
		engine.shaderManager.add("shaders/mesh");
		shader = engine.shaderManager.get("shaders/mesh");

		{
			ModelLoader loader;
			arm = std::move(loader.arm);
			animation = std::move(loader.animation);

			using NumberType = Engine::Gfx::NumberType;
			test.setFormat(Engine::Gfx::VertexFormat<3>{sizeof(Vertex), {
				{ .location = 0, .size = 3, .type = NumberType::Float32, .offset = offsetof(Vertex, pos) },
				{ .location = 1, .size = 4, .type = NumberType::UInt8, .offset = offsetof(Vertex, bones) },
				{ .location = 2, .size = 4, .type = NumberType::Float32, .offset = offsetof(Vertex, weights) },
			}});

			test.setVertexData(Engine::Gfx::Primitive::Triangles, loader.verts);
			test.setElementData(loader.indices);

		}

		bonesFinal.resize(arm.bones.size());

		glCreateBuffers(1, &ubo);
		glNamedBufferData(ubo, bonesFinal.size() * sizeof(bonesFinal[0]), nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo); // Bind index to ubo
		glUniformBlockBinding(shader->get(), 0, 1); // Bind uniform block to buffer index
	}

	AnimSystem::~AnimSystem() {
		glDeleteBuffers(1, &ubo);
	}

	void AnimSystem::updateAnim() {
		const auto nodeCount = arm.nodes.size();
		const auto tick = fmodf(clock() / 100.0f, animation.duration);

		for (const auto& seq : animation.channels) {
			const auto& interp = seq.interp(tick);
			arm.nodes[seq.nodeId].trans = glm::scale(glm::translate(glm::mat4{1.0f}, interp.pos) * glm::mat4_cast(interp.rot), interp.scale);
		}

		for (NodeId ni = 0; ni < nodeCount; ++ni) {
			auto& node = arm.nodes[ni];
			if (node.parentId >= 0) {
				node.total = arm.nodes[node.parentId].total * node.trans;
			} else {
				node.total = node.trans;
			}

			if (node.boneId >= 0) {
				bonesFinal[node.boneId] = node.total * arm.bones[node.boneId].offset;
			}
		}

		glNamedBufferSubData(ubo, 0, bonesFinal.size() * sizeof(bonesFinal[0]), bonesFinal.data());
	}

	void AnimSystem::render(const RenderLayer layer) {
		if (layer != RenderLayer::Debug) { return; }

		updateAnim();

		auto mvp = glm::ortho<float32>(0, 1920, 0, 1080, -10000, 10000);
		mvp = engine.camera.getProjection();
		mvp *= glm::scale(glm::mat4{1}, glm::vec3{1.0f / pixelsPerMeter});

		glUseProgram(shader->get());
		glUniformMatrix4fv(0, 1, GL_FALSE, &mvp[0][0]);

		test.draw();
	}
}
