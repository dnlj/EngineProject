// Game
#include <Game/systems/PhysicsSystem.hpp>
#include <Game/systems/MapSystem.hpp>
#include <Game/systems/UISystem.hpp>
#include <Game/UI/ZonePreview.hpp>
#include <Game/UI/TerrainPreview.hpp>

// Engine
#include <Engine/ArrayView.hpp>
#include <Engine/CommandManager.hpp>
#include <Engine/from_string.hpp>
#include <Engine/UI/Context.hpp>
#include <Engine/Window.hpp>


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
		template<auto MinValue, decltype(MinValue) MaxValue>
		constexpr auto Clamp = []<class T>(T& value) constexpr noexcept -> bool {
			static_assert(std::same_as<T, decltype(MinValue)>
				|| requires { requires std::same_as<typename T::rep, decltype(MinValue)>; },
				"Type mismatch between clamp bounds and argument type."
			);

			const auto before = value;
			value = std::clamp(value, static_cast<T>(MinValue), static_cast<T>(MaxValue));
			if (before != value) {
				ENGINE_WARN2("Invalid value. Clamping to {}.", value);
			}
			return false;
		};

		template<auto MinValue>
		constexpr auto Min = Clamp<MinValue, std::numeric_limits<decltype(MinValue)>::max()>;

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

				ENGINE_CONSOLE("{} = {}", args[0], str);
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
		return [&window](const auto& prev, const auto& current) {
			window.setSwapInterval(current);
		};
	}

	template<>
	ENGINE_INLINE auto makeOnChanged<&CVars::tn_gen_threads>(Game::EngineInstance& engine, Engine::Window& window) noexcept {
		return [&](const auto& prev, const auto& current) {
			ENGINE_SERVER_ONLY(engine.getWorld().getSystem<MapSystem>().generator().allocThreads(current));
			engine.getWorld().getSystem<UISystem>().getTerrainPreview()->generator().allocThreads(current);
		};
	}

	template<>
	ENGINE_INLINE auto makeOnChanged<&CVars::tn_gen_cache_target_size>(Game::EngineInstance& engine, Engine::Window& window) noexcept {
		return [&](const auto& prev, const auto& current) {
			const auto& cvars = Engine::getGlobalConfig().cvars;
			ENGINE_SERVER_ONLY(engine.getWorld().getSystem<MapSystem>().generator().setCacheSize(cvars.tn_gen_cache_target_size, cvars.tn_gen_cache_max_size));
			engine.getWorld().getSystem<UISystem>().getTerrainPreview()->generator().setCacheSize(cvars.tn_gen_cache_target_size, cvars.tn_gen_cache_max_size);
		};
	}

	template<>
	ENGINE_INLINE auto makeOnChanged<&CVars::tn_gen_cache_max_size>(Game::EngineInstance& engine, Engine::Window& window) noexcept {
		return [&](const auto& prev, const auto& current) {
			const auto& cvars = Engine::getGlobalConfig().cvars;
			ENGINE_SERVER_ONLY(engine.getWorld().getSystem<MapSystem>().generator().setCacheSize(cvars.tn_gen_cache_target_size, cvars.tn_gen_cache_max_size));
			engine.getWorld().getSystem<UISystem>().getTerrainPreview()->generator().setCacheSize(cvars.tn_gen_cache_target_size, cvars.tn_gen_cache_max_size);
		};
	}

	template<>
	ENGINE_INLINE auto makeOnChanged<&CVars::tn_gen_cache_timeout>(Game::EngineInstance& engine, Engine::Window& window) noexcept {
		return [&](const auto& prev, const auto& current) {
			ENGINE_SERVER_ONLY(engine.getWorld().getSystem<MapSystem>().generator().setCacheTimeout(current));
			engine.getWorld().getSystem<UISystem>().getTerrainPreview()->generator().setCacheTimeout(current);
		};
	}
}

void setupCommands(Game::EngineInstance& engine, Engine::Window& window) {
	auto& cm = engine.getCommandManager();
	cm.registerCommand("test_command", [](auto&){
		ENGINE_CONSOLE("This is a test command! {}", 123);
	});

	cm.registerCommand("phys_counts", [&engine](auto&){
		const auto& physSys = engine.getWorld().getSystem<PhysicsSystem>();
		const auto& physWorld = physSys.getPhysicsWorld();

		uint64 bodyCount = 0;
		uint64 fixtureCount = 0;
		for (auto* body = physWorld.GetBodyList(); body; body=body->GetNext()) {
			++bodyCount;
			for (auto* fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
				++fixtureCount;
			}
		}

		ENGINE_CONSOLE("Bodies: {}  Fixtures: {}", bodyCount, fixtureCount);
	});

	cm.registerCommand("zone_view", [&engine](auto&){
		auto& ctx = engine.getUIContext();
		const auto preview = ctx.createPanel<Game::UI::ZonePreview>(ctx.getRoot());
		preview->setPos({1920-512-16, 1080-512-16}); // TODO: rm
	});
	cm.exec("zone_view"); // TODO: rm
	
	// Build CVars
	{
		using namespace Validation;
		using std::chrono::nanoseconds;
		using std::chrono::microseconds;
		using std::chrono::milliseconds;
		using std::chrono::seconds;
		using std::chrono::minutes;
		using std::chrono::hours;

		#define X(Name, Side, Type, Default, Validators, ...) ENGINE_SIDE_ONLY(Side)(\
			cm.registerCommand(#Name, makeCVarFunc<&CVars::Name>( \
				makeValidateForSteps(Validators), \
				makeOnChanged<&CVars::Name>(engine, window) \
			)));
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
