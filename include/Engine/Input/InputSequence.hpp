#pragma once

// Engine
#include <Engine/StaticVector.hpp>
#include <Engine/Input/InputId.hpp>
#include <Engine/Input/InputState.hpp>


namespace Engine::Input {
	using InputSequence = StaticVector<InputId, 4>;
	using InputStateSequence = StaticVector<InputState, 4>; // TODO: move
}
