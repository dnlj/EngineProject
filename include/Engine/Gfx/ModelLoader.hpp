#pragma once

// Assimp
#include <assimp/Importer.hpp>

// Engine
#include <Engine/Gfx/AnimSeq.hpp>


namespace Engine::Gfx {
	// TODO: move
	class Animation {
		public:
			float32 duration;
			std::vector<Engine::Gfx::AnimSeq> channels;
	};

	using NodeId = int;
	using BoneId = int;
	using MeshId = int;
	struct Node {
		Node(NodeId parentId, BoneId boneId, const glm::mat4& bind)
			: parentId{parentId}
			, boneId{boneId}
			, trans{bind} {
		}

		#if ENGINE_DEBUG
			std::string name;
		#endif

		NodeId parentId = -1;
		BoneId boneId = -1; // Corresponding bone if any
		// NodeId childId = -1;
		// NodeId siblingId = -1;
		glm::mat4 trans; // parent -> local
		glm::mat4 total; // Total parent transform chain up to this point (includes this)
	};

	struct Bone {
		glm::mat4 offset;
	};

	// TODO: move
	class Armature {
		public:
			// Should be populated such that all ancestor nodes occur before child nodes. This should be done automatically because of how getNodeIndex is implemented.
			std::vector<Node> nodes;
			std::vector<Bone> bones;

			void clear() {
				nodes.clear();
				bones.clear();
			}
	};
	
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

	struct MeshDesc {
		uint32 offset = 0;
		uint32 count = 0;
		uint32 material = 0;
	};

	struct MeshInst {
		MeshId meshId;
		NodeId nodeId;
	};

	class ModelLoader {
		private:
			Assimp::Importer im;

		public: // TODO: private
			const struct aiScene* scene;

			std::vector<Vertex> verts;
			uint32 vertCount = 0;

			std::vector<uint32> indices;
			uint32 indexCount = 0;

			Engine::FlatHashMap<std::string_view, NodeId> nodeNameToId;
			std::vector<MeshDesc> meshes;

			std::vector<Animation> animations;
			std::vector<MeshInst> instances;
			Armature arm;

			bool skinned = false;

		public:
			ModelLoader();
			void load(const char* path);
			void clear();

		private:
			void init();
			void readMesh(const struct aiMesh* mesh);
			void readAnim(const struct aiAnimation* anim);
			NodeId getNodeId(const struct aiString& name);
			NodeId getNodeId(const struct aiNode* name);
			void build(aiNode* root, NodeId parentId);

	};
}
