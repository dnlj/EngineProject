#pragma once

// STD
#include <functional>

// Engine
#include <Engine/Input/Value.hpp>


namespace Engine::Input {
	using BindListener = std::function<void(Value curr, Value prev)>;
}
