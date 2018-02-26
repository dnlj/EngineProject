require "build/action_clean"

-------------------------------------------------------------------------------
-- Constants
-------------------------------------------------------------------------------
-- The name of this project. Should not be changed, it is here just for convenience.
PROJECT_NAME = "DungeonGame"

-------------------------------------------------------------------------------
-- The files and folders to delete when the "clean" action is run.
-------------------------------------------------------------------------------
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
	"./".. PROJECT_NAME .."Workspace.VC.VC.opendb"
}

-------------------------------------------------------------------------------
-- The main premake settings
-------------------------------------------------------------------------------
workspace(PROJECT_NAME .."Workspace")
	configurations {"Debug", "Debug_All", "Debug_Physics", "Debug_Graphics", "Release"}
	platforms {"Windows_x64"}
	characterset "Unicode"
	kind "ConsoleApp"
	language "C++"
	rtti "Off"
	warnings "Default"
	flags {
		"FatalWarnings",
		"MultiProcessorCompile",
	}
	targetdir "./bin/%{cfg.buildcfg}_%{cfg.platform}"
	objdir "./obj/%{prj.name}/%{cfg.buildcfg}_%{cfg.platform}"
	startproject(PROJECT_NAME)
	
	filter "action:vs*"
		systemversion "10.0.16299.0"
		buildoptions{
			"/std:c++latest", -- Use the latest version of C++
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
		defines {"RELEASE"}
		flags {"LinkTimeOptimization"}

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
		"./deps/Box2D/Box2D",
	}
	
	links {
		"glfw3.lib",
		"opengl32.lib",
		"SOIL.lib",
		"Box2D.lib",
	}
	
	filter {"platforms:Windows_x64", "configurations:Debug*"}
		libdirs {
			"./deps/glfw/build/src/Debug",
			"./deps/soil/projects/VC9/x64/Debug",
			"./deps/Box2D/Build/vs2017/bin/Debug",
		}
		
	filter {"platforms:Windows_x64", "configurations:Release*"}
		libdirs {
			"./deps/glfw/build/src/Release",
			"./deps/soil/projects/VC9/x64/Release",
			"./deps/Box2D/Build/vs2017/bin/Release",
		}

project(PROJECT_NAME)
	files {
		"./TODO.md",
		"./src/main.cpp",
		"./include/Game/**",
		"./src/Game/**",
	}

project(PROJECT_NAME .."Test")
	defines {"RUNNING_TESTS"}

	files {
		"./test/**",
	}
	
	includedirs {
		"./deps/googletest/googlemock/include",
		"./deps/googletest/googletest/include",
	}
	
	filter {"platforms:Windows_x64", "configurations:Debug*"}
		links {
			"gtestd.lib",
			"gmockd.lib",
		}
		
		libdirs {
			"./deps/googletest/googlemock/build/Debug",
			"./deps/googletest/googletest/build/Debug",
		}
		
	filter {"platforms:Windows_x64", "configurations:Release*"}
		links {
			"gtest.lib",
			"gmock.lib",
		}
		
		libdirs {
			"./deps/googletest/googlemock/build/Release",
			"./deps/googletest/googletest/build/Release",
		}
