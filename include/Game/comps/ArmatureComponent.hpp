#pragma once

// Engine
#include <Engine/Gfx/Armature.hpp>


namespace Game {
	class ArmatureComponent {
		public:
			auto& operator=(Engine::Gfx::Armature&& rhs) {
				arm = std::move(rhs);
				results.resize(arm.bones.size());
				return *this;
			}

			Engine::Gfx::Armature arm;

			// TODO: should this be private?
			/** The final accumulated bone transforms */
			std::vector<Engine::Gfx::Bone> results;
	};
}
