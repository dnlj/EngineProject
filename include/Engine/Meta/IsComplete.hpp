#pragma once

// STD
#include <type_traits>

// TODO: move this into Meta? Own namespace?
namespace Engine::Meta {
	template<class T, class = void>
	struct IsComplete : std::false_type {};

	template<class T>
	struct IsComplete<T, std::enable_if_t<(sizeof(T) > 0)>> : std::true_type {};
}
