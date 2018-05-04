local vs = require("build/vs_build")

newaction {
	trigger = "build",
	description = "Build the project.",
	execute = function()
		io.write("Building project...\n\n")
		
		if os.target() == "windows" then
			local buildCommon = vs:buildCommon({platform = "Windows_x64"})
			
			-- TODO: Update this to pull all config/platform combos from premake
			os.execute(vs.msbuild .." ".. PROJECT_NAME .."Workspace.sln " .. buildCommon .." /p:Configuration=Debug")
			io.write("\n")
			os.execute(vs.msbuild .." ".. PROJECT_NAME .."Workspace.sln " .. buildCommon .." /p:Configuration=Release")
		else
			-- TODO: 
			io.write("ERROR: TODO: Configure for non-windows")
		end
	end
}
