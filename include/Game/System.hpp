#pragma once

// Meta
#include <Meta/TypeSet/TypeSet.hpp>

// Engine
#include <Engine/SystemBase.hpp>


namespace Game {
	// TODO: rename to just System
	using SystemBase = Engine::SystemBase<class World>;
}
