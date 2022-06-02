#pragma once

// Engine
#include <Engine/ResourceManager2.hpp>
#include <Engine/Gfx/Buffer.hpp>


namespace Engine::Gfx {
	using BufferManager = ResourceManager2<Buffer>;
	using BufferRef = BufferManager::ResourceRef;
}
