#pragma once

// STD
#include <stdio.h>
#include <memory>

// Engine
#include <Engine/Types.hpp>
#include <Engine/Net/IPv4Address.hpp>


namespace Engine {
	class GlobalConfig {
		public:
			using UniqueFilePointer = std::unique_ptr<FILE, decltype(&fclose)>;
			GlobalConfig() = default;
			GlobalConfig(const GlobalConfig&) = delete;
			GlobalConfig(GlobalConfig&&) = delete;

			UniqueFilePointer log{stdout, [](FILE*)->int{ return 0; }};
			bool logColor = false;
			bool logTimeOnly = false;

			uint16 port = 0;
			Net::IPv4Address group = {};
	};

	namespace Detail { inline GlobalConfig globalConfig = {}; }
	template<bool Editable = false>
	auto& getGlobalConfig() {
		if constexpr (Editable) {
			return Detail::globalConfig;
		} else {
			return static_cast<const GlobalConfig&>(Detail::globalConfig);
		}
	}
};
