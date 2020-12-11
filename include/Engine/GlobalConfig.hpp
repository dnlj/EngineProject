#pragma once

// STD
#include <stdio.h>
#include <memory>


namespace Engine {
	class GlobalConfig {
		public:
			using UniqueFilePointer = std::unique_ptr<FILE, decltype(&fclose)>;
			GlobalConfig() = default;
			GlobalConfig(const GlobalConfig&) = delete;
			GlobalConfig(GlobalConfig&&) = delete;

			UniqueFilePointer log{stdout, &fclose};
			bool logColor = false;
			bool logTimeOnly = false;
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
