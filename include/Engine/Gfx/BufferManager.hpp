#pragma once

// Engine
#include <Engine/ResourceManager2.hpp>
#include <Engine/Gfx/Buffer.hpp>


namespace Engine::Gfx {
	class BufferManager : public ResourceManager2<Buffer> {
		using ResourceManager2::ResourceManager2;
	};
	using BufferRef = BufferManager::ResourceRef;
}
