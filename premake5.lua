require "premake5/actions"

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
	"./".. PROJECT_NAME ..".vcxproj",
	"./".. PROJECT_NAME ..".vcxproj.filters",
	"./".. PROJECT_NAME ..".vcxproj.user",
	"./".. PROJECT_NAME .."Test.vcxproj",
	"./".. PROJECT_NAME .."Test.vcxproj.filters",
	"./".. PROJECT_NAME .."Test.vcxproj.user",
	"./".. PROJECT_NAME .."Workspace.sln",
	"./".. PROJECT_NAME .."Workspace.VC.db",
	"./".. PROJECT_NAME .."Workspace.VC.VC.opendb"
}

-------------------------------------------------------------------------------
-- The main premake settings
-------------------------------------------------------------------------------
workspace(PROJECT_NAME .."Workspace")
	configurations {"Debug", "Release"}
	platforms {"Windows_x64"}
	characterset "Unicode"
	kind "ConsoleApp"
	language "C++"
	rtti "Off"
	warnings "Default"
	flags {"FatalWarnings"}
	targetdir "./bin/%{cfg.buildcfg}_%{cfg.platform}"
	objdir "./obj/%{prj.name}/%{cfg.buildcfg}_%{cfg.platform}"
	startproject(PROJECT_NAME)
	
	if (_ACTION == "vs2015" or _ACTION == "vs2017") then
		buildoptions{
			"/std:c++latest", -- Use the latest version of C++
			"/wd4996", -- Disable some warnings about things Visual Studio has taken apon itself to deem "deprecated"
		}
	end
	
	filter "platforms:Windows_x64"
        architecture "x64"
		defines {string.upper(PROJECT_NAME) .."_OS_WINDOWS"}
		
	filter "configurations:Debug"
		symbols "On"
		defines {"DEBUG"}
		
	filter "configurations:Release"
		optimize "Full"
		defines {"NDEBUG", "RELEASE"}

project(PROJECT_NAME)
	files {
		"./TODO.md",
		"./include/**",
		"./src/**",
	}
	
	debugdir "./src"
	
	includedirs {
		"./include",
		"./deps/glfw/include",
		"./deps/soil/src",
	}
	
	links {
		"glfw3.lib",
		"opengl32.lib",
		"SOIL.lib",
	}
	
	filter {"platforms:Windows_x64", "configurations:Debug"}
		libdirs {
			"./deps/glfw/build/src/Debug",
			"./deps/soil/projects/VC9/x64/Debug",
		}
		
	filter {"platforms:Windows_x64", "configurations:Release"}
		libdirs {
			"./deps/glfw/build/src/Release",
			"./deps/soil/projects/VC9/x64/Release",
		}
