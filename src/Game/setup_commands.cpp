// Engine
#include <Engine/CommandManager.hpp>
#include <Engine/from_string.hpp>
#include <Engine/ArrayView.hpp>

// TODO: rm
#include <type_traits>
#include <Engine/traits.hpp>

namespace {
	using namespace Game;

	// TODO: move to own file probably
	struct CompileString {
		const char* const data;
		consteval CompileString(const char* const data) noexcept : data{data} {}
		consteval std::string_view view() const noexcept { return data; }
		consteval explicit operator std::string_view() const noexcept { return view(); }
	};

	/**
	 * Validation functions used by cvars.
	 *
	 * Each validation function takes:
	 * 1. Reference to the new value.
	 * 2. Array of limitations specified in cvars.xpp. Usual in the form {min, max}.
	 * 
	 * And returns a bool where true = abort and false = continue. If a
	 * validation function signals an abort the old value is restored
	 */
	namespace Validation {
		template<auto Min, decltype(Min) Max>
		constexpr auto Clamp = [](Engine::AnyNumber auto& value) constexpr noexcept -> bool {
			puts("Clamp");
			const auto before = value;
			value = std::clamp(value, Min, Max);
			if (before != value) {
				ENGINE_WARN2("Invalid value. Clamping to {}.", value, before);
			}
			return false;
		};

		template<CompileString Msg>
		constexpr auto WarnIfDecimal = [](std::floating_point auto& value) constexpr noexcept -> bool {
			puts("WarnIfDecimal");
			if (value != std::trunc(value)) {
				ENGINE_WARN2(Msg);
			}
			return false;
		};
		
		template<CompileString Msg>
		constexpr auto WarnIfDecimal_Win32 = [](std::floating_point auto& value) constexpr noexcept -> bool {
			puts("WarnIfDecimal_Win32");
			if constexpr (ENGINE_OS_WINDOWS) {
				return WarnIfDecimal<Msg>(value);
			}
			return false;
		};
	}

	template<CompileString CVarName, class Func = Engine::None>
	consteval auto makeCVarFunc(Func&& validate = {}) {
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
						/* Filter out only the relevant cvar */\
						if constexpr (CVarName.view() == #Name) {\
							auto& cfg = Engine::getGlobalConfig<true>();\
							/* Perform validation if a function was given */\
							if constexpr (!std::same_as<Func, Engine::None>) {\
								using namespace Validation;\
								/*static_assert(requires { validate(cfg.cvars.Name, __VA_ARGS__); }, "Invalid cvar validation function");*/\
								const auto old = cfg.cvars.Name;\
								if (!fromString(args[1], cfg.cvars.Name)) { return false; }\
								if (validate(cfg.cvars.Name, __VA_ARGS__)) {\
									cfg.cvars.Name = old;\
								}\
							} else {\
								if (!fromString(args[1], cfg.cvars.Name)) { return false; }\
							}\
							return true;\
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
}

void setupCommands(Game::EngineInstance& engine) {
	auto& cm = engine.getCommandManager();
	const auto test = cm.registerCommand("test_command", [](auto&){
		ENGINE_CONSOLE("This is a test command! {}", 123);
	}); test;

	constexpr auto validate = [](auto& value, std::initializer_list<bool(*const)(decltype(value))> steps) ENGINE_INLINE {
		for (const auto& step : steps) {
			if (step(value)) { return true; }
		}
		return false;
	};
	////////////////////////////////////////////////////////////////////////////////////////////////
	// CVars
	////////////////////////////////////////////////////////////////////////////////////////////////
	// TODO: detect if vsync is supported, if so default to -1 instead of +1
	// TODO: how does our cvar setter handle negative numbers for unsigned types? we should be printing an error and then ignore.
	#define X(Name, ...) cm.registerCommand(#Name, makeCVarFunc<#Name>(validate));
	#include <Game/cvars.xpp>

	if constexpr (false) {
		std::vector<const char*> testData = {
			#include "../.private/testdata_ue"
		};

		for (auto cmd : testData) {
			cm.registerCommand(cmd, [](auto&){});
		}
	}
}
