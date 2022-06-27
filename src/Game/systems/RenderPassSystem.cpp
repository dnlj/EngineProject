#pragma once

// Engine
#include <Engine/Meta/ForEach.hpp>

// Game
#include <Game/World.hpp>
#include <Game/RenderLayer.hpp>
#include <Game/systems/all.hpp> // TODO: any way to get around this? not great for build times


namespace Game {
	void RenderPassSystem::update(float32 dt) {
		for (RenderLayer layer = {}; layer < RenderLayer::_count; ++layer) {
			Engine::Meta::ForEachIn<SystemsSet>::call([&]<class S>() ENGINE_INLINE {
				world.getSystem<S>().render(layer);
			});
		}
	}
}
