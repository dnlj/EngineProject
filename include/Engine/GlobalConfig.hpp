#pragma once

// Engine
#include <Engine/Types.hpp>
#include <Engine/Net/IPv4Address.hpp>
#include <Engine/Logger.hpp>


namespace Engine {
	class GlobalConfig {
		public:
			using UniqueFilePointer = std::unique_ptr<FILE, decltype(&fclose)>;
			GlobalConfig() = default;
			GlobalConfig(const GlobalConfig&) = delete;
			GlobalConfig(GlobalConfig&&) = delete;

			Engine::Logger logger;
			UniqueFilePointer log{stdout, [](FILE*)->int{ return 0; }};
			bool logColor = false;
			bool logTimeOnly = false;

			// Command line only
			uint16 port = 0;
			Net::IPv4Address group = {};

			// TODO: shouldn't be including "Game" files in Engine
			struct CVars {
				using nanoseconds = std::chrono::nanoseconds;
				using microseconds = std::chrono::microseconds;
				using milliseconds = std::chrono::milliseconds;
				using seconds = std::chrono::seconds;
				using minutes = std::chrono::minutes;
				using horus = std::chrono::hours;

				#define X(Name, Side, Type, Default, ...) ENGINE_SIDE_ONLY(Side)(Type Name = Type{Default});
				#include <Game/cvars.xpp>
			} cvars;
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
