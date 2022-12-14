#pragma once

// STD
#include <charconv>
#include <concepts>

// Engine
#include <Engine/Net/Net.hpp>


namespace Engine::CommandLine {
	template<class T>
	class ArgumentConverter {
		public: bool operator()(const std::string& str, T& storage) {
			static_assert(ENGINE_TMP_FALSE(T), "Unable to find ArgumentConverter for T");
		}
	};

	template<std::integral T>
	class ArgumentConverter<T> {
		public: bool operator()(const std::string& str, T& storage) {
			return std::from_chars(str.data(), str.data() + str.size(), storage).ec == std::errc{};
		}
	};
	
	template<std::floating_point T>
	class ArgumentConverter<T> {
		public: bool operator()(const std::string& str, T& storage) {
			return std::from_chars(str.data(), str.data() + str.size(), storage).ec == std::errc{};
		}
	};
	
	template<>
	class ArgumentConverter<bool> {
		public: bool operator()(const std::string& str, bool& storage) {
			storage = (str.size() > 0) && (str[0] != '0') && (str[0] != 'f') && (str[0] != 'F');
			return true;
		}
	};

	template<>
	class ArgumentConverter<std::string> {
		public: bool operator()(const std::string& str, std::string& storage) {
			storage = str;
			return true;
		}
	};

	template<>
	class ArgumentConverter<Net::IPv4Address> {
		public: bool operator()(const std::string& str, Net::IPv4Address& storage) {
			// TODO: this should have some error checking????
			storage = Net::hostToAddress(str);
			return true;
		}
	};
}
