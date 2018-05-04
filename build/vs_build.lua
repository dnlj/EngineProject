-- Visual Studio info
local vs = {}
vs.year = "2017"
vs.toolVersion = "15.0"
vs.platformToolset = "v141"
vs.sdkVersion = "10.0.16299.0"
vs.version = "Community"
vs.msbuild = '"C:/Program Files (x86)/Microsoft Visual Studio/'.. vs.year ..'/'.. vs.version ..'/MSBuild/'.. vs.toolVersion ..'/Bin/msbuild.exe"'
vs.devenv = '"C:/Program Files (x86)/Microsoft Visual Studio/'.. vs.year ..'/'.. vs.version ..'/Common7/IDE/devenv.exe"'
vs.cmake = "Visual Studio 15 2017 Win64"

-- Other defaults
vs.platform = "x64"
vs.verbosity = "minimal"

function vs:buildCommon(args)
	local args = args or {}
	local verbosity = args.verbosity or self.verbosity
	local toolVersion = args.toolVersion or self.toolVersion
	local platform = args.platform or self.platform
	local platformToolset = args.platformToolset or self.platformToolset
	local toolVersion = args.toolVersion or self.toolVersion
	local sdkVersion = args.sdkVersion or self.sdkVersion
	
	return "/t:Build"..
	" /verbosity:".. verbosity ..
	" /toolsversion:".. toolVersion ..
	" /p:Platform=".. platform ..
	" /p:PlatformToolset=".. platformToolset ..
	" /p:VisualStudioVersion=".. toolVersion ..
	" /p:WindowsTargetPlatformVersion=".. sdkVersion
end
	
return vs
