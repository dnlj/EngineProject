#pragma once

// STD
#include <charconv>
#include <concepts>

// Engine
#include <Engine/Net/Net.hpp>


namespace Engine::CommandLine {
	template<class T>
	class ArgumentConverter {
		public: void operator()(const std::string& str, T& storage) {
			static_assert(!std::is_same_v<T, T>, "Unable to find ArgumentConverter for T");
		}
	};

	template<std::integral T>
	class ArgumentConverter<T> {
		public: void operator()(const std::string& str, T& storage) {
			std::from_chars(str.data(), str.data() + str.size(), storage);
		}
	};
	
	template<std::floating_point T>
	class ArgumentConverter<T> {
		public: void operator()(const std::string& str, T& storage) {
			std::from_chars(str.data(), str.data() + str.size(), storage);
		}
	};
	
	template<>
	class ArgumentConverter<bool> {
		public: void operator()(const std::string& str, bool& storage) {
			storage = (str.size() > 0) && (str[0] != '0') && (str[0] != 'f') && (str[0] != 'F');
		}
	};

	template<>
	class ArgumentConverter<std::string> {
		public: void operator()(const std::string& str, std::string& storage) {
			storage = str;
		}
	};

	template<>
	class ArgumentConverter<Net::IPv4Address> {
		public: void operator()(const std::string& str, Net::IPv4Address& storage) {
			storage = Net::hostToAddress(str);
		}
	};
}
