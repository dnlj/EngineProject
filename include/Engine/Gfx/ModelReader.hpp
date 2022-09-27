#pragma once

// Assimp
#include <assimp/Importer.hpp>

// Engine
#include <Engine/Gfx/Animation.hpp>
#include <Engine/Gfx/Armature.hpp>


namespace Engine::Gfx {
	struct Vertex {
		Vertex() {};

		glm::vec3 pos;
		glm::u16vec2 uv;

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

	struct MaterialDesc {
		std::string name;
		std::string path;
		int count = 0;
	};

	class ModelReader {
		public:
			struct Results {
				std::vector<Vertex> verts;
				std::vector<uint32> indices;
				std::vector<MeshDesc> meshes;
				std::vector<MaterialDesc> materials;
				std::vector<Animation> animations;
				std::vector<MeshInst> instances;
				Armature arm;
				bool skinned;
			};

		private:
			Assimp::Importer im;
			const struct aiScene* scene;
			Results res;
			uint32 vertCount = 0;
			uint32 indexCount = 0;
			Engine::FlatHashMap<std::string_view, NodeId> nodeNameToId;

		public:
			ModelReader();
			bool load(const char* path);
			const Results& results() const noexcept { return res; }
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
