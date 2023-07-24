// Engine
#include <Engine/CommandManager.hpp>
#include <Engine/from_string.hpp>

namespace {
	// TODO: move to own file probably
	struct CompileString {
		const char* const data;
		consteval CompileString(const char* const data) noexcept : data{data} {}
		consteval std::string_view view() const noexcept { return data; }
		consteval explicit operator std::string_view() const noexcept { return view(); }
	};
}

void setupCommands(Game::EngineInstance& engine) {
	auto& cm = engine.getCommandManager();
	const auto test = cm.registerCommand("test_command", [](auto&){
		ENGINE_CONSOLE("This is a test command! {}", 123);
	}); test;

	constexpr auto cvar = []<CompileString CVarName, class Func = Engine::None>(Func&& validate = {}) consteval {
		return [validate=std::forward<Func>(validate)](Engine::CommandManager& cm){
			const auto& args = cm.args();
			if (args.size() == 1) {
				std::string str = [&args]() ENGINE_INLINE_REL -> std::string {
					using fmt::to_string;
					#define X(Name, ...)\
						if constexpr (CVarName.view() == #Name) {\
							const auto& cfg = Engine::getGlobalConfig();\
							if (args[0] == CVarName.view()) { return to_string(cfg.cvars.Name); }\
						}
					#include <Game/cvars.xpp>
					return {};
				}();

				if (str.empty()) {
					// TODO: error
					ENGINE_WARN2("TODO: error");
					ENGINE_DEBUG_BREAK;
				}

				ENGINE_CONSOLE("Get ({}) {} = {}", args[0], args.size(), str);
			} else {
				ENGINE_CONSOLE("Set ({}) {}", args[0], args.size());
				const auto good = [&args, &validate]() -> bool {
					using Engine::fromString;
					#define X(Name, Type, Default, ...)\
						if constexpr (CVarName.view() == #Name) {\
							if (args[0] == CVarName.view()) {\
								auto& cfg = Engine::getGlobalConfig<true>();\
								if (!fromString(args[1], cfg.cvars.Name)) { return false; }\
								if constexpr (!std::same_as<Func, Engine::None>) {\
									static_assert(requires { validate(cfg.cvars.Name, __VA_ARGS__); }, "Invalid cvar validation function");\
									validate(cfg.cvars.Name, __VA_ARGS__);\
								}\
								return true;\
							}\
						}
					#include <Game/cvars.xpp>
					return false;
				}();

				if (!good) {
					ENGINE_WARN2("Unable to set cvar \"{}\" to  \"{}\"", args[0], args[1]);
				}
			}
		};
	};
	
	constexpr auto validate_clamp = []<class V>(V& value, const V& min, const V& max) constexpr {
		ENGINE_LOG2("Before: {}", value);
		value = std::clamp<V>(value, min, max);
		ENGINE_LOG2("After: {}", value);
	};

	// TODO: move to file and add helper to remove duplicate name
	#define CM_REGISTER_CVAR(Name, ...) cm.registerCommand(Name, cvar.template operator()<Name>(__VA_ARGS__))
	CM_REGISTER_CVAR("net_packet_rate_min");
	CM_REGISTER_CVAR("net_packet_rate_max");

	// TODO: detect if vsync is supported, if so default to -1 instead of +1
	// TODO: frametime should warn if setting decimal number on windows (time != int(time))
	// TODO: how does our cvar setter handle negative numbers for unsigned types? we should be printing an error and then ignore.
	// TODO: need to add fromString for bools
	CM_REGISTER_CVAR("frametime");
	CM_REGISTER_CVAR("vsync", validate_clamp);
	#undef CM_REGISTER_COMMAND

	if constexpr (false) {
		std::vector<const char*> testData = {
			#include "../.private/testdata_ue"
		};

		for (auto cmd : testData) {
			//cm.registerCommand(cmd, cvar());
		}
	}
}
