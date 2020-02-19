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
					msg.warn("\rUnable to delete ", match, " with error: ", err:gsub("[\r\n]", " "), "\n")
				else
					msg.success("\rSuccessfully deleted ", match, "\n")
				end
			end
		end
	end
}
