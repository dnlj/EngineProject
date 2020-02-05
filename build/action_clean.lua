newaction {
	trigger = "clean",
	description = "Clean the workspace",
	execute = function()
		for _, p in pairs(action_clean_patterns) do
			local matches = os.match(p)
			for _, match in pairs(matches) do
				io.write("Deleting ".. match)
				local suc, err
				if os.isfile(match) then
					suc, err = os.remove(match)
				else
					suc, err = os.rmdir(match)
				end
				
				if not suc then
					term.pushColor(term.warningColor)
					io.write("\rUnable to delete ", match, " with error: ", err:gsub("[\r\n]", " "), "\n")
					term.popColor()
				else
					term.pushColor(term.infoColor)
					io.write("\rSuccessfully deleted ", match, "\n")
					term.popColor()
				end
			end
		end
	end
}
