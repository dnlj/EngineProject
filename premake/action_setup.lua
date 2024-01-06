local function downloadAndExtractZip(url, out, force)
	local temp = out ..".zip"

	if os.isdir(out) then
		if not force then
			print("Already exists, skipping: ".. out)
			return
		else
			if not os.remove(temp) then
				error("Unable to forcibly remove existing directory, skipping: ".. out)
				return
			end
		end
	end

	local err = http.download(url, temp, {})
	if err ~= "OK" then
		error("Unable to download \"".. url .."\": ".. err)
	else
		zip.extract(temp, out)
	end

	if not os.remove(temp) then
		error("Unable to delete temporary file: ".. temp)
	end
end

local commands = {}
commands.download = function()
	for i, entry in pairs(SETUP_CONFIG) do
		print("\nDownloading \"".. entry.name .."\"...")
		os.mkdir(path.getdirectory(entry.dir))
		downloadAndExtractZip(entry.url, entry.dir)
	end
end

commands.clean = function()
	for i, entry in pairs(SETUP_CONFIG) do
		print("\nRemoving \"".. entry.name .."\"...")
		os.rmdir(entry.dir)
	end
end

newaction {
	trigger = "setup",
	description = "Setup project tools and non-code dependancies",
	execute = function()
		-- Validate config
		assert(SETUP_CONFIG, "SETUP_CONFIG must be defined")
		for _, entry in pairs(SETUP_CONFIG) do
			assert(entry.name, "SETUP_CONFIG entry is missing field: name")
			assert(entry.url, "SETUP_CONFIG entry \"".. entry.name .."\" is missing field: url")
			assert(entry.dir, "SETUP_CONFIG entry \"".. entry.name .."\" is missing field: dir")
		end

		assert(#_ARGS == 1, "Multiple arguments are not supported")
		local cmdstr = _ARGS[1]
		local cmd = commands[cmdstr]

		if cmd then
			cmd()
		else
			error("Unknown setup command: ".. cmdstr)
		end

		print("\nSetup ".. table.concat(_ARGS, " ") .." complete.")
	end
}
