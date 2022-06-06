#pragma once

// Engine
#include <Engine/ResourceManager.hpp>
#include <Engine/Gfx/Buffer.hpp>


namespace Engine::Gfx {
	class BufferManager : public ResourceManager<Buffer> {
		using ResourceManager::ResourceManager;
	};
}
