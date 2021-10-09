#pragma once

// STD
#include <functional>
#include <type_traits>

// Engine
#include <Engine/ECS/Entity.hpp>
#include <Engine/Input/Value.hpp>


namespace Engine::Input {
	using ActionId = uint8;
	using ActionListener = std::function<bool(ECS::Entity ent, ActionId aid, Value curr, Value prev)>;

	template<class T>
	concept IsActionListener = std::is_convertible_v<T, ActionListener>;
}
