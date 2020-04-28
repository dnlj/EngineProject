#pragma once

// STD
#include <functional>
#include <type_traits>

// Engine
#include <Engine/Input/ActionId.hpp>
#include <Engine/Input/Value.hpp>


namespace Engine::Input {
	using ActionListener = std::function<bool(Value curr, Value prev)>;

	template<class T>
	concept IsActionListener = std::is_convertible_v<T, ActionListener>;
}
