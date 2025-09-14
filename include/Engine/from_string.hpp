#pragma once

// STD
#include <charconv>


namespace Engine {
	template<class T>
	requires std::convertible_to<std::string_view, T>
	bool fromString(std::string_view str, T& value) {
		value = str;
		return true;
	}

	template<class T>
	requires (std::integral<T> || std::floating_point<T>)
	ENGINE_INLINE bool fromString(std::string_view str, T& value) {
		static_assert(!std::is_const_v<T>, "An output parameter cannot be const.");
		const auto& [_, err] = std::from_chars(std::to_address(str.cbegin()), std::to_address(str.cend()), value);
		return err == std::errc{};
	}


	template<class T, class R>
	ENGINE_INLINE bool fromString(std::string_view str, std::chrono::duration<T, R>& value) {
		T temp{};

		if (fromString(str, temp)) {
			value = std::chrono::duration<T, R>{temp};
			return true;
		}

		return false;
	}
}
