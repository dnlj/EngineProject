local vs = require("build/vs_build")
local args = {...}
local depsDir = args[1]

local PACKAGES = {}

local function moveFile(fromFolder, toFolder, fileName)
	os.mkdir(toFolder)
	local succ, err = os.rename(fromFolder .. fileName, toFolder .. fileName)
	
	if not succ then
		print(err)
	end
end

local function moveFolder(from, to)
	local temp = "_TEMP_".. math.random(0, 10000) .."_/"
	
	local succ, err = os.rename(from, temp)
	if succ then
		os.rmdir(from)
		os.mkdir(to)
		os.rmdir(to)
		
		succ, err = os.rename(temp, to)
		
		if not succ then
			print(err)
		end
	else
		print(err)
	end
end

local function buildMSBuild(fileName, args)
	io.write("\n")
	os.execute(vs.msbuild .." ".. fileName .." ".. vs:buildCommon() .." ".. args)
end

local function buildCMakeProjectWithMSBuild(workingDir, buildDir, buildFile, cmakeArgs, msBuildArgs, filesToMove)
	local oldDir = os.getcwd()
	
	os.mkdir(workingDir .. buildDir)
	os.chdir(workingDir .. buildDir)
	
	-- CMake
	io.write("\n")
	os.execute(
		'cmake -G "'.. vs.cmake ..'" '..
		table.concat(cmakeArgs, " ") ..
		' ..'
	)
	
	-- MSBuild
	local buildCommon = table.concat(msBuildArgs, " ") .." ".. vs:buildCommon()
	
	buildMSBuild(buildFile, buildCommon .." /p:Configuration=Debug")
	buildMSBuild(buildFile, buildCommon .." /p:Configuration=Release")
	
	-- Move files
	os.chdir(oldDir .. workingDir)
	
	for k,v in pairs(filesToMove) do
		moveFile(table.unpack(v))
	end
	
	os.chdir(oldDir)
end

--------------------------------------------------------------------------------
-- Packages
--------------------------------------------------------------------------------
PACKAGES["glfw"] = {
	name = "Graphics Library Framework (GLFW)",
	url = "https://github.com/glfw/glfw/archive/master.zip",

	postExtract = function(uid)
		local dir = depsDir .. uid .."/"
		moveFolder(dir .."glfw-master/", dir)
	end,
	
	build = function(uid)
		local dir =  depsDir .. uid .."/"
		
		if os.target() == "windows" then
			buildCMakeProjectWithMSBuild(dir, "build/", "GLFW.sln", {
				"-DGLFW_BUILD_EXAMPLES=OFF",
				"-DGLFW_BUILD_TESTS=OFF",
				"-DGLFW_BUILD_DOCS=OFF",
			}, {}, {
				{"./build/src/Debug/", "./lib/debug/", "glfw3.lib"},
				{"./build/src/Release/", "./lib/release/", "glfw3.lib"},
			})
		else
			-- TODO: 
			io.write("ERROR: TODO: Configure non-windows builds")
		end
	end,
}

PACKAGES["glm"] = {
	name = "OpenGL Mathematics (GLM)",
	url = "https://github.com/g-truc/glm/archive/master.zip",

	postExtract = function(uid)
		local dir = depsDir .. uid .."/"
		
		moveFolder(dir .."glm-master/", dir .."include/")
		
		for k,v in pairs(os.matchdirs(dir .."include/*")) do
			if string.sub(v, -3) ~= "glm" then
				os.rmdir(v)
			end
		end
		
		for k,v in pairs(os.matchfiles(dir .."include/*")) do
			os.remove(v)
		end
		
		for k,v in pairs(os.matchfiles(dir .."include/**CMakeLists.txt")) do
			os.remove(v)
		end
	end,
	
	build = function(uid)
	end,
}

PACKAGES["soil"] = {
	name = "Simple OpenGL Image Library (SOIL)",
	url = "http://www.lonesock.net/files/soil.zip",

	postExtract = function(uid)
		local dir = depsDir .. uid .."/"
		moveFolder(dir .."Simple OpenGL Image Library/", dir)
	end,
	
	build = function(uid)
		local dir = depsDir .. uid .."/"
		
		if os.target() == "windows" then
			local buildDir = dir .."/projects/VC9/"
			-- Upgrade
			os.execute(vs.devenv ..' '.. buildDir ..'SOIL.sln /upgrade')
			
			-- Build
			buildMSBuild(buildDir .."SOIL.sln", "/p:Configuration=Release")
			buildMSBuild(buildDir .."SOIL.sln", "/p:Configuration=Debug")
			
			-- Move files
			local libDir = dir .."lib/"
			os.rmdir(libDir)
			
			moveFile(buildDir .."x64/Debug/", libDir .."debug/", "SOIL.lib")
			moveFile(buildDir .."x64/Release/", libDir .."release/", "SOIL.lib")
		else
			-- TODO: 
			io.write("ERROR: TODO: Configure non-windows builds")
		end
	end,
}

PACKAGES["box2d"] = {
	name = "Box2D",
	url = "https://github.com/erincatto/Box2D/archive/master.zip",

	postExtract = function(uid)
		local dir = depsDir .. uid .."/"
		moveFolder(dir .."Box2D-master/Box2D/", dir)
	end,
	
	build = function(uid)
		local dir = depsDir .. uid .."/"
		local premake = '"'.. os.getcwd() ..'/premake5"'
		
		if os.target() == "windows" then
			-- Create project files
			local oldDir = os.getcwd()
			os.chdir(dir)
			os.execute(premake ..' vs2017')
			
			local buildFile = './Build/vs2017/Box2D.sln'
			
			-- Build
			buildMSBuild(buildFile, "/p:Configuration=Debug")
			buildMSBuild(buildFile, "/p:Configuration=Release")

			-- Move files
			moveFile("./Build/vs2017/bin/Debug/", "./lib/debug/", "Box2D.lib")
			moveFile("./Build/vs2017/bin/Release/", "./lib/release/", "Box2D.lib")
			
			-- Make include folder
			moveFolder("./Box2D/", "./include/Box2D/")
			
			os.chdir(oldDir)
		else
			-- TODO: 
			io.write("ERROR: TODO: Configure non-windows builds")
		end
	end,
}

PACKAGES["googletest"] = {
	name = "Google Test",
	url = "https://github.com/google/googletest/archive/master.zip",

	postExtract = function(uid)
		local dir = depsDir .. uid .."/"
		moveFolder(dir .."googletest-master/", dir)
	end,
	
	build = function(uid)
		local dir =  depsDir .. uid .."/"
		local cmakeCommon = {"-Dgtest_force_shared_crt=ON"}
		
		if os.target() == "windows" then
			buildCMakeProjectWithMSBuild(dir, "googletest/build/", "gtest.sln", cmakeCommon, {}, {
				{"./googletest/build/Debug/", "./lib/debug/", "gtestd.lib"},
				{"./googletest/build/Release/", "./lib/release/", "gtest.lib"},
			})
			
			buildCMakeProjectWithMSBuild(dir, "googlemock/build/", "gmock.sln", cmakeCommon, {}, {
				{"./googlemock/build/Debug/", "./lib/debug/", "gmockd.lib"},
				{"./googlemock/build/Release/", "./lib/release/", "gmock.lib"},
			})
		else
			-- TODO: 
			io.write("ERROR: TODO: Configure non-windows builds")
		end
	end,
}

PACKAGES["meta"] = {
	name = "Meta",
	url = "https://github.com/dnlj/Meta/archive/master.zip",

	postExtract = function(uid)
		local dir = depsDir .. uid .."/"
		moveFolder(dir .."Meta-master/", dir)
	end,
	
	build = function(uid)
	end,
}

return PACKAGES
