--------------------------------------------------------------------------------
-- Constants / Globals
--------------------------------------------------------------------------------
PROJECT_NAME = "DungeonGame"
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
	"./docs",

	-- Conan
	"conan_build",
	".temp.conanfile.*"
}

--------------------------------------------------------------------------------
-- Conan Settings
--------------------------------------------------------------------------------
CONAN_USER_HOME = os.getcwd()

CONAN_REMOTES = {
	-- ["conan-center"] = "https://api.bintray.com/conan/conan/conan-center",
	-- ["bincrafters"] = "https://api.bintray.com/conan/bincrafters/public-conan",
}

CONAN_PACKAGES = { -- TODO: Name?
	["requires"] = {
		"box2d/022d9eccfcbebe339f1df3a17d205110d9623a80@dnlj/wobbly",
		"glm/0.9.9.7@dnlj/wobbly",
		"imgui/1.75@dnlj/wobbly",
		"implot/master@dnlj/wobbly",
		"meta/master@dnlj/wobbly",
		"premake5/latest@dnlj/wobbly",
		"robin_hood/3.5.0@dnlj/wobbly",
		"soil/latest@dnlj/wobbly", -- TODO: Look into soil2 or other image loading lib. We dont use any of the opengl features of soil.
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
		options = {},
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
	flags {
		"FatalWarnings",
		"MultiProcessorCompile",
	}
	targetdir "./bin/%{cfg.buildcfg}_%{cfg.platform}"
	objdir "./obj/%{prj.name}/%{cfg.buildcfg}_%{cfg.platform}"
	startproject(PROJECT_NAME)
	defines {
		"GLM_FORCE_PURE" -- TODO: Remove. See https://github.com/g-truc/glm/issues/841
	}

	filter "action:vs*"
		buildoptions{
			"/wd4996", -- Disable some warnings about things Visual Studio has taken apon itself to deem "deprecated"
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
	}
	
	debugdir "src"

	filter "configurations:Debug*"
		conan_setup_build_info(CONAN_BUILD_INFO["debug"])
	filter "configurations:Release*"
		conan_setup_build_info(CONAN_BUILD_INFO["release"])
	filter {}

	includedirs {
		"include",
	}

	links {
		"opengl32",
		"Ws2_32",
	}

	libdirs {
	}

--------------------------------------------------------------------------------
-- Client
--------------------------------------------------------------------------------
project(PROJECT_NAME .."Client")
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
project(PROJECT_NAME .."Server")
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
-- Test
--------------------------------------------------------------------------------
--project(PROJECT_NAME .."Test")
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
