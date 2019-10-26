#pragma once

// Engine
#include <Engine/StaticVector.hpp>
#include <Engine/InputState.hpp>


namespace Engine {
	using InputSequence = StaticVector<Input, 4>;
	using InputStateSequence = StaticVector<InputState, 4>; // TODO: move
}
