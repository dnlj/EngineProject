#pragma once

// Engine
#include <Engine/ResourceManager.hpp>


namespace Engine::Gfx {
	class Animation;

	class AnimationManager : public ResourceManager<Animation> {
		using ResourceManager::ResourceManager;
	};
}
