// Assimp
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

// Engine
#include <Engine/Gfx/Mesh.hpp>
#include <Engine/ArrayView.hpp>

// Game
#include <Game/systems/AnimSystem.hpp>


namespace Game {
	AnimSystem::AnimSystem(SystemArg arg) : System{arg} {
		engine.shaderManager.add("shaders/mesh");
		shader = engine.shaderManager.get("shaders/mesh");

		constexpr char fileName[] = "assets/char5.fbx";
		//constexpr char fileName[] = "assets/char.glb";

		Assimp::Importer im;
		const aiScene* scene = im.ReadFile(fileName, aiProcessPreset_TargetRealtime_Fast);

		struct Vertex {
			Vertex() {};

			glm::vec3 pos;

			// TODO: instead of invalid bone id we can just check if weight == 0, can do this in shader also
			uint8 bones[4];
			float32 weights[4] = {};

			int nextBoneIndex() const noexcept {
				for (int i = 0; i < std::size(bones); ++i) {
					if (weights[i] == 0) { return i; }
				}
				ENGINE_DEBUG_ASSERT(false, "Attempting to add to many bones to vertex.");
				return 0;
			}

			void addBone(uint8 bone, float32 weight) noexcept {
				const auto i = nextBoneIndex();
				bones[i] = bone;
				weights[i] = weight;
			}
		};

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

			//float scale = 123;
			//ENGINE_LOG("UnitScaleFactor: ", scene->mMetaData->Get("UnitScaleFactor", scale), " ", scale);
		}

		for (const auto* anim : Engine::ArrayView{scene->mAnimations, scene->mNumAnimations}) {
			ENGINE_LOG(
				"\n\tName: ", anim->mName.C_Str(),
				"\n\tTicks/s: ", anim->mTicksPerSecond,
				"\n\tChannels:", anim->mNumChannels,
				"\n\tMeshChannels:", anim->mNumMeshChannels,
				""
			);

			for (const auto* chan : Engine::ArrayView{anim->mChannels, anim->mNumChannels}) {
				ENGINE_LOG("\t\t", chan->mNodeName.C_Str());
			}
		}

		std::vector<Vertex> verts;
		std::vector<uint32> indices;
		Engine::FlatHashMap<std::string_view, NodeIndex> nodeToIndex; // Map from an Assimp aiNode to index into nodes array

		auto cvtMat = [](const aiMatrix4x4& m) { return glm::mat4{ // Convert to column major and glm
			{m.a1, m.b1, m.c1, m.d1},
			{m.a2, m.b2, m.c2, m.d2},
			{m.a3, m.b3, m.c3, m.d3},
			{m.a4, m.b4, m.c4, m.d4},
		};};

		auto getNodeIndex = [&](const aiString& name) {
			auto impl = [&](auto& self, const aiString& name, const aiNode* node) -> NodeIndex {
				auto found = nodeToIndex.find(std::string_view{name.data, name.length});
				if (found == nodeToIndex.end()) {
					if (!node) {
						node = scene->mRootNode->FindNode(name);
					}

					NodeIndex parent = -1;
					// TODO: need to stop at mesh or mesh parent node. I think bones dont NEED to be in one of these places but in practice they will. If they arent we can say the model is ill formed and should be fixed on the model side, not in code
					if (node->mParent && node->mParent != scene->mRootNode) {
						parent = self(self, node->mParent->mName, node->mParent);
					}

					found = nodeToIndex.emplace(std::string_view{name.data, name.length}, static_cast<NodeIndex>(nodes.size())).first;
					nodes.emplace_back(parent, -1, cvtMat(node->mTransformation));

					#if ENGINE_DEBUG
						nodes.back().name = found->first;
					#endif

					

					ENGINE_LOG("Build Node: ", found->second, " - ", node->mName.C_Str(), " - ", cvtMat(node->mTransformation));
				}

				return found->second;
			};
			return impl(impl, name, nullptr);
		};

		// TODO: i think we really need to walk the scene, there may be multiple instances of a mesh
		ENGINE_ASSERT(scene->mNumMeshes == 1, "Invalid number of meshes in file"); // TODO: handle error, dont assert
		for(const auto* mesh : Engine::ArrayView{scene->mMeshes, scene->mNumMeshes}) {
			{
				indices.clear();
				verts.clear();
				verts.resize(mesh->mNumVertices);
				indices.resize(mesh->mNumFaces * 3);

				const auto estNodes = nodes.size() + mesh->mNumBones * 2; // Just a guess, no way to get true number without walking `scene->mRootNode`
				nodes.reserve(estNodes);
				nodeToIndex.reserve(estNodes);
			}
			
			ENGINE_ASSERT(mesh->mNumBones <= 255, "To many bones in model"); // TODO: handle error, dont assert
			for (uint i = 0; i < mesh->mNumBones; ++i) {
				const auto& bone = mesh->mBones[i];
				auto boneIndex = getNodeIndex(bone->mName);
				for (const auto& weight : Engine::ArrayView{bone->mWeights, bone->mNumWeights}) {
					verts[weight.mVertexId].addBone(boneIndex, weight.mWeight);
				}
				nodes[boneIndex].offset = cvtMat(bone->mOffsetMatrix);
				//nodes[boneIndex].offset = glm::mat4{1};
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

			using NumberType = Engine::Gfx::NumberType;
			test.setFormat(Engine::Gfx::VertexFormat<3>{sizeof(Vertex), {
				{ .location = 0, .size = 3, .type = NumberType::Float32, .offset = offsetof(Vertex, pos) },
				{ .location = 1, .size = 4, .type = NumberType::UInt8, .offset = offsetof(Vertex, bones) },
				{ .location = 2, .size = 4, .type = NumberType::Float32, .offset = offsetof(Vertex, weights) },
			}});

			test.setVertexData(Engine::Gfx::Primitive::Triangles, verts);
			test.setElementData(indices);
		}

		nodes.shrink_to_fit();

		if (nodes.size() > 100) {
			ENGINE_WARN("To many bones in model. Clamping. ", fileName);
			ENGINE_DIE; // TODO: clamp number of bones
		}

		nodesFinal.resize(nodes.size());
		ENGINE_LOG("*** Nodes: ", nodes.size());

		glCreateBuffers(1, &ubo);
		glNamedBufferData(ubo, nodesFinal.size() * sizeof(nodesFinal[0]), nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo); // Bind index to ubo
		glUniformBlockBinding(shader->get(), 0, 1); // Bind uniform block to buffer index
	}

	AnimSystem::~AnimSystem() {
		glDeleteBuffers(1, &ubo);
	}

	void AnimSystem::updateAnim() {
		const auto nodeCount = nodes.size();

		for (NodeIndex ni = 0; ni < nodeCount; ++ni) {
			auto& node = nodes[ni];
			if (node.parentIndex >= 0) {
				nodesFinal[ni] = nodesFinal[node.parentIndex] * node.trans;
			} else {
				nodesFinal[ni] = node.trans;
			}
		}

		for (NodeIndex ni = 0; ni < nodeCount; ++ni) {
			auto& node = nodes[ni];
			nodesFinal[ni] *= node.offset;
		}

		// TODO: ideally we would only upload bones, not all nodes. How do others handle this?
		// as far as i could tell both the tutorials i looked at just ignore non-bone nodes, which would lead to incorrect behaviour.
		glNamedBufferSubData(ubo, 0, nodesFinal.size() * sizeof(nodesFinal[0]), nodesFinal.data());
	}

	void AnimSystem::render(const RenderLayer layer) {
		if (layer != RenderLayer::Debug) { return; }

		updateAnim();

		auto mvp = glm::ortho<float32>(0, 1920, 0, 1080, -10000, 10000);
		mvp = engine.camera.getProjection();

		//float32 scale = 1.0f/ abs((sin(clock() / 1000.0f) * 10000));
		//mvp *= glm::scale(glm::mat4{1}, glm::vec3{scale});

		mvp *= glm::scale(glm::mat4{1}, glm::vec3{1.0f / pixelsPerMeter});

		glUseProgram(shader->get());
		glUniformMatrix4fv(0, 1, GL_FALSE, &mvp[0][0]);

		test.draw();
	}
}
