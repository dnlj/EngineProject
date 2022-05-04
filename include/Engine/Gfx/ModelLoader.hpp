#pragma once

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

	// TODO: move
	class ModelLoader {
		private:
		public: // TODO: private
			const struct aiScene* scene;
			std::vector<Vertex> verts;
			std::vector<uint32> indices;
			Engine::FlatHashMap<std::string_view, NodeId> nodeToIndex; // Map from an Assimp aiNode to index into nodes array

			Animation animation;
			Armature arm;

			ModelLoader();

		private:
			void readMesh(const struct aiMesh* mesh);
			void readAnim(const struct aiAnimation* anim);
			NodeId getNodeId(const struct aiString& name, const struct aiNode* node = nullptr);

	};
}
