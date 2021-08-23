#pragma once

// Engine
#include <Engine/Meta/ForEach.hpp>

// Game
#include <Game/World.hpp>
#include <Game/RenderLayer.hpp>


namespace Game {
	void RenderPassSystem::run(float32 dt) {
		for (RenderLayer layer = {}; layer < RenderLayer::_count; ++layer) {
			Engine::Meta::ForEachIn<SystemsSet>::call([&]<class S>() ENGINE_INLINE {
				world.getSystem<S>().render(layer);
			});
		}
	}
}
