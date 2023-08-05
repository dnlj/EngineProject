// Engine
#include <Engine/CommandManager.hpp>
#include <Engine/from_string.hpp>
#include <Engine/ArrayView.hpp>

// TODO: rm
#include <type_traits>
#include <Engine/traits.hpp>

namespace {
	using namespace Game;

	// TODO: rm
	//using CVars = Engine::GlobalConfig::CVars;
	//
	//template<auto CVar>
	//concept IsValidCVar = requires (const CVars& cvars) { cvars.*CVar; };
	//
	//// Specialize to call when a cvar is changed: template<> constexpr auto onChanged<&CVars::r_vsync> = [](){ ... }
	//template<auto CVar>
	//requires IsValidCVar<CVar>
	//constexpr auto onChanged = [](const auto&...) consteval noexcept {};
	//
	//template<>
	//constexpr auto onChanged<&CVars::r_vsync> = [](){
	//	puts("onChanged r_vsync");
	//	// TODO: update swap interval
	//};

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

		template<Engine::CompileString Msg>
		constexpr auto WarnIfDecimal = [](std::floating_point auto& value) constexpr noexcept -> bool {
			puts("WarnIfDecimal");
			if (value != std::trunc(value)) {
				ENGINE_WARN2(Msg);
			}
			return false;
		};
		
		template<Engine::CompileString Msg>
		constexpr auto WarnIfDecimal_Win32 = [](std::floating_point auto& value) constexpr noexcept -> bool {
			puts("WarnIfDecimal_Win32");
			if constexpr (ENGINE_OS_WINDOWS) {
				return WarnIfDecimal<Msg>(value);
			}
			return false;
		};
	}

	template<auto CVarMember, class Validate, class OnChanged>
	consteval auto makeCVarFunc(Validate&& validate, OnChanged&& onChanged) {
		return [validate=std::forward<Validate>(validate), onChanged=std::forward<OnChanged>(onChanged)](Engine::CommandManager& cm){
			const auto& args = cm.args();
			if (args.size() == 1) {
				using fmt::to_string;
				const auto& cfg = Engine::getGlobalConfig();
				const auto str = to_string(cfg.cvars.*CVarMember);

				if (str.empty()) {
					// TODO: error
					ENGINE_WARN2("TODO: error");
					ENGINE_DEBUG_BREAK;
				}

				ENGINE_CONSOLE("Get ({}) {} = {}", args[0], args.size(), str);
			} else {
				ENGINE_CONSOLE("Set ({}) {}", args[0], args.size());
				using Engine::fromString;
				auto& cvar = Engine::getGlobalConfig<true>().cvars.*CVarMember;
				const auto old = cvar;

				if (!fromString(args[1], cvar)) {
					ENGINE_WARN2("Unable to set cvar \"{}\" to  \"{}\"", args[0], args[1]);
					return;
				}

				if (validate(cvar)) {
					cvar = old;
				} else {
					onChanged(old, cvar);
				}
			}
		};
	};

	template<class... Steps>
	constexpr auto makeValidateForSteps(Steps... steps) {
		return [steps...](auto& value){
			return (steps(value) || ...);
		};
	}
}

void setupCommands(Game::EngineInstance& engine) {
	auto& cm = engine.getCommandManager();
	const auto test = cm.registerCommand("test_command", [](auto&){
		ENGINE_CONSOLE("This is a test command! {}", 123);
	}); test;
	
	// Build CVars
	// TODO: Just move into makeCVarFunc
	constexpr auto validate = [](auto& value, std::initializer_list<bool(*const)(decltype(value))> steps) ENGINE_INLINE {
		for (const auto& step : steps) {
			if (step(value)) { return true; }
		}
		return false;
	};
	constexpr auto onChanged = [](auto&...){};

	{
		using namespace Validation;
		#define X(Name, Type, Default, Validators) \
			cm.registerCommand(#Name, makeCVarFunc<&Engine::GlobalConfig::CVars::Name>( \
				makeValidateForSteps(Validators), \
				onChanged \
			));
		#include <Game/cvars.xpp>
	}

	// Test Data
	if constexpr (false) {
		std::vector<const char*> testData = {
			#include "../.private/testdata_ue"
		};

		for (auto cmd : testData) {
			cm.registerCommand(cmd, [](auto&){});
		}
	}
}
