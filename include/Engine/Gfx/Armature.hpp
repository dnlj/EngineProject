#pragma once


namespace Engine::Gfx {
	using NodeId = int32;
	using BoneId = int32;

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

	class Armature {
		public:
			// Should be populated such that all ancestor nodes occur before child nodes.
			// This should be done automatically because of how getNodeIndex is implemented.
			std::vector<Node> nodes;

			/** Inverse bind pose for each bone. */
			std::vector<glm::mat4> boneOffsets;

			/** The final accumulated bone transforms. */
			std::vector<glm::mat4> results;

			void reserve(size_t cap) {
				nodes.reserve(cap);
				boneOffsets.reserve(cap);
			}

			void clear() {
				nodes.clear();
				boneOffsets.clear();
				results.clear();
			}

			void finalize() {
				nodes.shrink_to_fit();
				boneOffsets.shrink_to_fit();
				results.resize(boneOffsets.size());
			}
	};
}
