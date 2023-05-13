--------------------------------------------------------------------------------
-- Constants / Globals
--------------------------------------------------------------------------------
PROJECT_NAME = "EngineProject"
CONFIG_TYPE_STR = '%{string.lower(string.match(cfg.buildcfg, "^([^_]+)"))}'

--------------------------------------------------------------------------------
-- Clean settings
--------------------------------------------------------------------------------
CLEAN_PATTERNS = {
	-- Visual Studio
	".vs",
	"*.vcxproj*",
	"*.sln",
	"*.VC.db",
	"*.VC.VC.opendb",

	-- Build Artifacts
	"./bin",
	"./obj",
	"./docs/generated",

	-- Conan
	"conan_build",
	".temp.conanfile.*"
}

--------------------------------------------------------------------------------
-- Conan Settings
--------------------------------------------------------------------------------
CONAN_USER_HOME = os.getcwd()

function conan_setup(cfg)
	if not cfg or not CONAN_BUILD_INFO or not CONAN_BUILD_INFO[cfg] then return end
	CONAN_BUILD_INFO[cfg].setup()
end

CONAN_REMOTES = {
	-- ["conan-center"] = "https://api.bintray.com/conan/conan/conan-center",
	-- ["bincrafters"] = "https://api.bintray.com/conan/bincrafters/public-conan",
}

CONAN_PACKAGES = {
	["requires"] = {
		"box2d/022d9eccfcbebe339f1df3a17d205110d9623a80@dnlj/wobbly",
		"glm/0.9.9.7@dnlj/wobbly",
		--"imgui/1.82@dnlj/wobbly",
		--"imgui-node-editor/master@dnlj/wobbly",
		--"implot/0.9@dnlj/wobbly",
		"meta/master@dnlj/wobbly",
		"pcg/master@dnlj/wobbly",
		"premake5/latest@dnlj/wobbly",
		"robin_hood/master@dnlj/wobbly",
		"soil_littlstar/master@dnlj/wobbly",
		"freetype/VER-2-10-4@dnlj/wobbly",
		"harfbuzz/2.8.1@dnlj/wobbly",
		"fmtlib/8.0.1@dnlj/wobbly",
		--"soil/latest@dnlj/wobbly",
		"assimp/5.2.3@dnlj/wobbly",

	},
	["generators"] = {
		"premake5",
	}
}

CONAN_PROFILES = {
	common = {
		build = false,
		includes = {},
		settings = {
			arch = "x86_64",
		},
		options = {
			["harfbuzz:with_freetype"] = true,
		},
		env = {},
	},
	release = {
		build = true,
		includes = {"common"},
		settings = {
			build_type = "Release",
		},
		options = {},
		env = {},
	},
	debug = {
		build = true,
		includes = {"common"},
		settings = {
			build_type = "Debug",
		},
		options = {},
		env = {},
	},
}

--------------------------------------------------------------------------------
--
--------------------------------------------------------------------------------
require "vstudio"
require "premake/message"
require "premake/action_clean"
require "premake/action_build"
require "premake/action_tests"
require "premake/action_conan"

premake.override(premake.vstudio.vc2010.elements, "user", function(base, cfg)
	local calls = base(cfg)
	table.insert(calls, function(cfg) premake.w("<ShowAllFiles>true</ShowAllFiles>") end)
	return calls
end)

--------------------------------------------------------------------------------
-- The main premake settings
--------------------------------------------------------------------------------
workspace(PROJECT_NAME .."Workspace")
	configurations {"Debug", "Debug_All", "Debug_Physics", "Debug_Graphics", "Release", "Release_Debug"}
	platforms {"Windows_x64"}
	characterset "Unicode"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++latest"
	systemversion "latest"
	rtti "Off"
	warnings "Default"
	targetdir "./bin/%{cfg.buildcfg}_%{cfg.platform}"
	objdir "./obj/%{prj.name}/%{cfg.buildcfg}_%{cfg.platform}"
	startproject(PROJECT_NAME)

	flags {
		"FatalWarnings",
		"MultiProcessorCompile",
	}

	defines {
		"GLM_FORCE_PURE", -- TODO: Remove. Link dead. Think it had something to do with constexpr See https://github.com/g-truc/glm/issues/841
		"HB_NO_MT", -- Harfbuzz: Disable thread safety
		"HB_LEAN", -- Harfbuzz: Disable lots of non critical and deprecated functions
		"HB_MINI", -- Harfbuzz: Disable AAT and legacy fonts
		"__OPTIMIZE_SIZE__" -- Harfbuzz: Seems to only effect sorting function selection
	}

	filter "action:vs*"
		buildoptions{
			"/wd4996", -- Disable some warnings about things Visual Studio has taken upon itself to deem deprecated
			"/wd4103", -- Work around for MSVC bug. TODO: remove when fixed - https://developercommunity.visualstudio.com/t/Warning-C4103-in-Visual-Studio-166-Upda/1057589
			--"/w14061", -- Not enabled because of enum count cases. Enable: missing switch case for enum
			--"/w14062", -- Not enabled because of enum count cases. Enable: missing switch case for enum, and no default
			--"/w14127", -- Not enabled because of inconsistent results when mixing both constexpr and non-constexpr statements in the same statement. Enable: use of constant expression in non-constexpr context
			"/w14132", -- Enable: const object should be initialized
			"/w14189", -- Enable: local variable is initialized but not referenced
			"/w14456", -- Enable: declaration hides previous local declaration
			"/w14457", -- Enable: declaration hides function parameter
			"/w14700", -- Enable: uninitialized local variable used
			"/w14701", -- Enable: potentially uninitialized local variable 'name' used
			--"/w14710", -- Enable: function marked as inline not inlined
			"/w14714", -- Enable: function marked as __forceinline not inlined -- TODO: does this also work for `[[msvc::force_inline]]`?
			"/w15038", -- Enable: out of order initialization warnings. Bugs related to this can be tricky to track down.
			"/diagnostics:column", -- Include column in error messages
			--"/diagnostics:caret", -- Enable more detailed error reporting
		}

	filter "platforms:Windows_x64"
        architecture "x64"
		defines {
			"ENGINE_OS_WINDOWS",
			"WIN32_LEAN_AND_MEAN",
			"NOMINMAX",
			"ENGINE_BASE_PATH=R\"(".. os.getcwd() .. ")\"",
		}

	filter "configurations:Debug*"
		symbols "On"
		defines {"DEBUG"}
		inlining "Explicit"
		editandcontinue "Off" -- As of Visual Studio 16.7 MSVC uses /ZI (capital i) by default which prevents /Ob1 (__forceinline) from working. See https://developercommunity.visualstudio.com/t/major-debug-performance-regression-ob1-no-longer-w/1177277#T-N1188009

	filter "configurations:Debug_All"
		defines {"DEBUG_ALL"}

	filter {"configurations:Debug_Physics or configurations:Debug_All"}
		defines {"DEBUG_PHYSICS"}

	filter {"configurations:Debug_Graphics or configurations:Debug_All"}
		defines {"DEBUG_GRAPHICS"}

	filter "configurations:Release*"
		symbols "Off"
		optimize "Speed"
		inlining "Auto"
		defines {"NDEBUG"}
		flags {"LinkTimeOptimization"}

	filter {"configurations:Release*", "action:vs*"}
		inlining "Default" -- To avoid D9025
		buildoptions {"/Ob3"}

	filter "configurations:Release_Debug"
		symbols "On"
		-- TODO: look into MSVC /Zo

--------------------------------------------------------------------------------
-- Engine
--------------------------------------------------------------------------------
--project(PROJECT_NAME .."Engine")
--	kind "None"

-- The engine files are put in the workspace since Game, Engine, and Test all use them.
project("*")
	files {
		"include/Engine/**",
		"src/Engine/**",
		"include/glloadgen/**",
		"src/glloadgen/**",
		"shaders/**",
		"src/pch.cpp",
		"include/pch.hpp",
	}
	
	pchheader "pch.hpp"
	pchsource "src/pch.cpp"
	forceincludes "pch.hpp"

	debugdir(os.getcwd())
	filter "platforms:Windows*"
		links {
			"opengl32",
			"Ws2_32",
			"Imm32",
			"hid",
		}
	filter "configurations:Debug*"
		conan_setup("debug")
	filter "configurations:Release*"
		conan_setup("release")
	filter {}
	
	includedirs {
		"include",
	}

	links {
	}

	libdirs {
	}

--------------------------------------------------------------------------------
-- Client
--------------------------------------------------------------------------------
project("Client")
	uuid "6E25C6C1-DA3B-C457-23B3-4F798F0895DF"
	files {
		"TODO.md",
		"src/main.cpp",
		"include/Game/**",
		"src/Game/**",
	}

	defines {
		"ENGINE_SIDE=ENGINE_SIDE_CLIENT",
	}

--------------------------------------------------------------------------------
-- Server
--------------------------------------------------------------------------------
project("Server")
	uuid "863A9FE6-F250-9D7C-3BC8-289EA71D6E04"
	files {
		"TODO.md",
		"src/main.cpp",
		"include/Game/**",
		"src/Game/**",
	}

	defines {
		"ENGINE_SIDE=ENGINE_SIDE_SERVER",
	}

--------------------------------------------------------------------------------
-- Bench
--------------------------------------------------------------------------------
project("Bench")
	uuid "71611229-1162-4773-AC04-0B86A3FE1AD0"
	kind "ConsoleApp"
	flags { "ExcludeFromBuild" }
	
	files {
		"bench/**",
		"include/bench/**",
		"src/bench/**",
	}
	
	includedirs {
		"include/bench/**",
	}

	defines {
		"ENGINE_SIDE=ENGINE_SIDE_SERVER",
	}
--------------------------------------------------------------------------------
-- Test
--------------------------------------------------------------------------------
--project("Test")
--	defines {"RUNNING_TESTS"}
--
--	files {
--		"./test/**",
--	}
--
--	includedirs {
--		"./deps/googletest/googlemock/include",
--		"./deps/googletest/googletest/include",
--	}
--
--	libdirs {
--		"./deps/googletest/lib/".. CONFIG_TYPE_STR,
--	}
--
--	filter {"platforms:Windows_x64", "configurations:Debug*"}
--		links {
--			"gtestd.lib",
--			"gmockd.lib",
--		}
--
--	filter {"platforms:Windows_x64", "configurations:Release*"}
--		links {
--			"gtest.lib",
--			"gmock.lib",
--		}
