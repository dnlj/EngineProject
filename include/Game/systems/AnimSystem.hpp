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
			using NodeIndex = int;
			using BoneIndex = int;
			struct Node {
				Node(NodeIndex parentIndex, BoneIndex boneIndex, const glm::mat4& bind)
					: parentIndex{parentIndex}
					, boneIndex{boneIndex}
					, bind{bind}
					, trans{bind} {
				}

				#if ENGINE_DEBUG
					std::string name;
				#endif

				NodeIndex parentIndex = -1;
				BoneIndex boneIndex = -1;
				glm::mat4 bind; // The default (bind) transform
				glm::mat4 trans; // parent -> local
				glm::mat4 offset = glm::mat4{1};
			};

			struct Bone {
				glm::mat4 offset;
			};

			// Should be populated such that all ancestor nodes occur before child nodes. This should be done automatically because of how getNodeIndex is implemented.
			std::vector<Node> nodes;

			//std::vector<Bone>

			// TODO: change this to only store final bone transforms, nodes can store parent transforms.
			// TODO: we really only need bones here. not all nodes
			std::vector<glm::mat4> nodesFinal;

			Animation animation;

		public:
			AnimSystem(SystemArg arg);
			~AnimSystem();
			void updateAnim();
			void render(const RenderLayer layer);
	};
}
