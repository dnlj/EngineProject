newaction {
	trigger = "test",
	description = "Run the tests.",
	execute = function()
		io.write("Running tests...\n")
		
		if os.target() == "windows" then
			local files = os.matchfiles("bin/**/*Test.exe")
			
			for k,v in pairs(files) do
				io.write("\nRunning test (".. k .."/".. #files ..") (".. v ..")\n")
				os.execute('""./'.. v ..'""')
			end
		else
			-- TODO: 
			io.write("ERROR: TODO: Configure for non-windows")
		end
	end
}
