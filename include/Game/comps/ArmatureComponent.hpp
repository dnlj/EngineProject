#pragma once

// Engine
#include <Engine/Gfx/Armature.hpp>


namespace Game {
	class ArmatureComponent : public Engine::Gfx::Armature {
		public:
			using Armature::Armature;
			using Armature::operator=;
	};
}
