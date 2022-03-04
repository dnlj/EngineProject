#pragma once

// STD
#include <string>
#include <type_traits>


namespace Engine::Utility {
	/**
	 * Gets the contents of the file located at @p path.
	 * @param[in] path The path to the file.
	 * @return The contents of the file as a string.
	 */
	std::string readFile(const std::string& path);

	// TODO: use C++20 concept std::integral instead of static assert - as of VS16.6.0 it compiles but breaks intellisense
	template<class T>
	constexpr bool isPowerOfTwo(T i) {
		static_assert(std::is_integral_v<T>, "T must be integral type.");
		return !(i & (i - 1));
	}
}
