#pragma once

// Engine
#include <Engine/Gfx/resources.hpp>


namespace Engine::Gfx {
	class Animation;

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

			/** Inverse bind pose for each bone. Converts from mesh space to bone space. */
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
				results.resize(boneOffsets.size());
			}

			void apply(const Animation& anim, float32 tick);
			void rebuild();
	};
}
