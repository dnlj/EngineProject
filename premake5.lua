--------------------------------------------------------------------------------
-- Constants / Globals
--------------------------------------------------------------------------------
PROJECT_NAME = "DungeonGame"
CONFIG_TYPE_STR = '%{string.lower(string.match(cfg.buildcfg, "^([^_]+)"))}'

--------------------------------------------------------------------------------
-- Conan Settings
--------------------------------------------------------------------------------
CONAN_USER_HOME = os.getcwd()

-- TODO: CONAN_REMOTES = {a, b, c}

CONAN_PACKAGES = { -- TODO: Name?
	["requires"] = {
		"glm/0.9.9.7@user/channel",
		--"soil2/1.11@user/channel",
		--"test/1.2.3@user/channel",
	},
	["generators"] = {
		"premake",
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
-- TODO: rename build folder
require "build/printTable"
require "build/action_clean"
require "build/action_deps"
require "build/action_build"
require "build/action_tests"
require "build/action_conan"

--------------------------------------------------------------------------------
-- The files and folders to delete when the "clean" action is run.
--------------------------------------------------------------------------------
-- TODO: move these with the rest of the constants/globals
action_clean_directories = {
	"./.vs",
	"./bin",
	"./obj",
	"./docs",
}

action_clean_files = {
	-- Visual Studio files
	"./".. PROJECT_NAME ..".vcxproj",
	"./".. PROJECT_NAME ..".vcxproj.filters",
	"./".. PROJECT_NAME ..".vcxproj.user",
	"./".. PROJECT_NAME .."Test.vcxproj",
	"./".. PROJECT_NAME .."Test.vcxproj.filters",
	"./".. PROJECT_NAME .."Test.vcxproj.user",
	"./".. PROJECT_NAME .."Engine.vcxproj",
	"./".. PROJECT_NAME .."Engine.vcxproj.filters",
	"./".. PROJECT_NAME .."Engine.vcxproj.user",
	"./".. PROJECT_NAME .."Workspace.sln",
	"./".. PROJECT_NAME .."Workspace.VC.db",
	"./".. PROJECT_NAME .."Workspace.VC.VC.opendb",

	-- TODO: conan files
}


--------------------------------------------------------------------------------
-- The main premake settings
--------------------------------------------------------------------------------
workspace(PROJECT_NAME .."Workspace")
	configurations {"Debug", "Debug_All", "Debug_Physics", "Debug_Graphics", "Release"}
	platforms {"Windows_x64"}
	characterset "Unicode"
	kind "ConsoleApp"
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
		defines {string.upper(PROJECT_NAME) .."_OS_WINDOWS"}

	filter "configurations:Debug*"
		symbols "On"
		defines {"DEBUG"}

	filter "configurations:Debug_All"
		defines {"DEBUG_ALL"}

	filter {"configurations:Debug_Physics or configurations:Debug_All"}
		defines {"DEBUG_PHYSICS"}

	filter {"configurations:Debug_Graphics or configurations:Debug_All"}
		defines {"DEBUG_GRAPHICS"}

	filter "configurations:Release*"
		optimize "Full"
		defines {"NDEBUG"}
		flags {"LinkTimeOptimization"}

--------------------------------------------------------------------------------
-- Engine
--------------------------------------------------------------------------------
project(PROJECT_NAME .."Engine")
	kind "None"

-- The engine files are put in the workspace since Game, Engine, and Test all use them.
project("*")
	files {
		"./include/Engine/**",
		"./src/Engine/**",
		"./include/glloadgen/**",
		"./src/glloadgen/**",
	}

	debugdir "./src"

	includedirs {
		"./include",
		"./deps/glfw/include",
		"./deps/soil/src",
		"./deps/glm/include",
		"./deps/box2d/include",
		"./deps/meta/include",
		"./deps/robin_hood/src/include",
	}

	links {
		"glfw3",
		"opengl32",
		"SOIL",
		"Box2D",
	}

	libdirs {
		"./deps/glfw/lib/".. CONFIG_TYPE_STR,
		"./deps/soil/lib/".. CONFIG_TYPE_STR,
		"./deps/box2d/lib/".. CONFIG_TYPE_STR,
	}

--------------------------------------------------------------------------------
-- Game
--------------------------------------------------------------------------------
project(PROJECT_NAME)
	files {
		"./TODO.md",
		"./src/main.cpp",
		"./include/Game/**",
		"./src/Game/**",
		"./deps/imgui/**",
	}

	includedirs {
		"./deps/imgui",
	}

--------------------------------------------------------------------------------
-- Test
--------------------------------------------------------------------------------
project(PROJECT_NAME .."Test")
	defines {"RUNNING_TESTS"}

	files {
		"./test/**",
	}

	includedirs {
		"./deps/googletest/googlemock/include",
		"./deps/googletest/googletest/include",
	}

	libdirs {
		"./deps/googletest/lib/".. CONFIG_TYPE_STR,
	}

	filter {"platforms:Windows_x64", "configurations:Debug*"}
		links {
			"gtestd.lib",
			"gmockd.lib",
		}

	filter {"platforms:Windows_x64", "configurations:Release*"}
		links {
			"gtest.lib",
			"gmock.lib",
		}
