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
};
