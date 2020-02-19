newaction {
	trigger = "clean",
	description = "Clean the workspace",
	execute = function()
		if not CLEAN_PATTERNS then
			msg.error("No clean patterns found.\n")
			return
		end

		for _, p in pairs(CLEAN_PATTERNS) do
			local matches = os.match(p)
			for _, match in pairs(matches) do
				msg.print("Deleting ", match)

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
