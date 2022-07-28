#pragma once

// Assimp
#include <assimp/Importer.hpp>

// Engine
#include <Engine/Gfx/Animation.hpp>
#include <Engine/Gfx/Armature.hpp>


namespace Engine::Gfx {
	// TODO: move into own file
	using MeshId = int32;
	
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
