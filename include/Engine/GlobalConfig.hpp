#pragma once

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

			// TODO: maybe these shouldnt be here. Move to EngineInstance with ConsoleManager
			uint16 port = 0;
			Net::IPv4Address group = {};
			float32 sendRateMax = 256;
			float32 sendRateMin = 8;
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
