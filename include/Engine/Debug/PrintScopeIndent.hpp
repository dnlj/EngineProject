#pragma once

// Engine
#include <Engine/FlatHashMap.hpp>

namespace Engine::Debug {
	class PrintScopeIndent {
		private:
			inline static Engine::FlatHashMap<std::string_view, int32> depths{};
			std::string_view scope;

		public:
			template<class... Ts>
			PrintScopeIndent(std::string_view scope, std::string_view format, const Ts&... ts)
				: scope{scope} {
				auto& depth = depths[scope];
				fmt::print("{:{}s}", "", 2*depth);
				fmt::vprint(format, fmt::make_format_args(ts...));
				++depth;
			}

			~PrintScopeIndent() {
				--depths[scope];
			}
	};
}

#if ENGINE_DEBUG
	#define ENGINE_DEBUG_PRINT_SCOPE(...) Engine::Debug::PrintScopeIndent _engine_debug_print_scope_##__LINE__{__VA_ARGS__};
#else
	#define ENGINE_DEBUG_PRINT_SCOPE(...)
#endif
