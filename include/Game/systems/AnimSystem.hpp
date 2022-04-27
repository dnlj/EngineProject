#pragma once

// Engine
#include <Engine/Gfx/AnimSeq.hpp>

// Game
#include <Game/System.hpp>


namespace Game {
	// TODO: move
	class Animation {
		public:
			float32 duration;
			std::vector<Engine::Gfx::AnimSeq> channels;
	};

	class AnimSystem : public System {
		private:
			Engine::Gfx::Mesh test;
			Engine::ShaderRef shader;
			GLuint ubo;

			// TODO: add separate Bones vector that has the boneOffset - that way we only upload the values we need and max the total bones we can have
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

			// Should be populated such that all ancestor nodes occur before child nodes. This should be done automatically because of how getNodeIndex is implemented.
			std::vector<Node> nodes;
			std::vector<Bone> bones;

			std::vector<glm::mat4> bonesFinal;

			Animation animation;

		public:
			AnimSystem(SystemArg arg);
			~AnimSystem();
			void updateAnim();
			void render(const RenderLayer layer);
	};
}
