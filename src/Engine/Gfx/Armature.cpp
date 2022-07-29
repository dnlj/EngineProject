// Engine
#include <Engine/Gfx/Armature.hpp>
#include <Engine/Gfx/Animation.hpp>


namespace Engine::Gfx {
	void Armature::apply(const Animation& anim, float32 tick) {
		for (const auto& seq : anim.channels) {
			const auto& interp = seq.interp(tick);
			nodes[seq.nodeId].trans = glm::scale(glm::translate(glm::mat4{1.0f}, interp.pos) * glm::mat4_cast(interp.rot), interp.scale);
		}

		rebuild();
	}

	void Armature::rebuild() {
		for (auto& node : nodes) {
			if (node.parentId >= 0) {
				node.total = nodes[node.parentId].total * node.trans;
			} else {
				node.total = node.trans;
			}

			if (node.boneId >= 0) {
				results[node.boneId] = node.total * boneOffsets[node.boneId];
			}
		}
	}
}
