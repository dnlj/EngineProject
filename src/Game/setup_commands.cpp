// Engine
#include <Engine/CommandManager.hpp>
#include <Engine/from_string.hpp>
#include <Engine/ArrayView.hpp>
#include <Engine/Window.hpp>

// TODO: rm
#include <type_traits>
#include <Engine/traits.hpp>

namespace {
	using namespace Game;
	using CVars = Engine::GlobalConfig::CVars;
	
	template<auto CVar>
	concept IsValidCVar = requires (const CVars& cvars) { cvars.*CVar; };

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
			const auto before = value;
			value = std::clamp(value, Min, Max);
			if (before != value) {
				ENGINE_WARN2("Invalid value. Clamping to {}.", value, before);
			}
			return false;
		};

		template<Engine::CompileString Msg>
		constexpr auto WarnIfDecimal = [](std::floating_point auto& value) constexpr noexcept -> bool {
			if (value != std::trunc(value)) {
				ENGINE_WARN2(Msg);
			}
			return false;
		};
		
		template<Engine::CompileString Msg>
		constexpr auto WarnIfDecimal_Win32 = [](std::floating_point auto& value) constexpr noexcept -> bool {
			if constexpr (ENGINE_OS_WINDOWS) {
				return WarnIfDecimal<Msg>(value);
			}
			return false;
		};
	}

	// Can't be consteval anymore because onChanged captures the engine instance.
	template<auto CVarMember, class Validate, class OnChanged>
	constexpr auto makeCVarFunc(Validate&& validate, OnChanged&& onChanged) {
		return [validate=std::forward<Validate>(validate), onChanged=std::forward<OnChanged>(onChanged)](Engine::CommandManager& cm){
			const auto& args = cm.args();
			if (args.size() == 1) {
				using fmt::to_string;
				const auto& cfg = Engine::getGlobalConfig();
				const auto str = to_string(cfg.cvars.*CVarMember);

				if (str.empty()) {
					// don't think fmt::to_string can fail?
					ENGINE_WARN2("Unable to convert to string");
					ENGINE_DEBUG_BREAK;
				}

				ENGINE_CONSOLE("{} = {}", args[0], args.size(), str);
			} else {
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

	// Specialize to call when a cvar is changed
	template<auto CVar>
	requires IsValidCVar<CVar>
	ENGINE_INLINE auto makeOnChanged(Game::EngineInstance& engine, Engine::Window& window) noexcept {
		return [](const auto& prev, const auto& current) noexcept {};
	}

	template<>
	ENGINE_INLINE auto makeOnChanged<&CVars::r_vsync>(Game::EngineInstance& engine, Engine::Window& window) noexcept {
		return [&window](const auto& prev, const auto& current){
			window.setSwapInterval(current);
		};
	}
}

void setupCommands(Game::EngineInstance& engine, Engine::Window& window) {
	auto& cm = engine.getCommandManager();
	const auto test = cm.registerCommand("test_command", [](auto&){
		ENGINE_CONSOLE("This is a test command! {}", 123);
	}); test;
	
	// Build CVars
	{
		using namespace Validation;
		#define X(Name, Type, Default, Validators) \
			cm.registerCommand(#Name, makeCVarFunc<&CVars::Name>( \
				makeValidateForSteps(Validators), \
				makeOnChanged<&CVars::Name>(engine, window) \
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
