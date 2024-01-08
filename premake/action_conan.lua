-- TODO: Doc each of these
assert(CONAN_ROOT, "CONAN_HOME must be defined")
assert(CONAN_PACKAGES, "CONAN_PACKAGES must be defined")
assert(CONAN_PROFILES, "CONAN_PROFILES must be defined")
assert(CONAN_EXE, "CONAN_EXE must be defined")

local commands = {}

-- TODO: make user configurable?
CONAN_HOME = path.join(CONAN_ROOT, ".conan")
CONAN_BUILD_DIR = path.join(CONAN_ROOT, "conan_build")
CONAN_RECIPES_DIR = path.join(CONAN_ROOT, "conan_recipes")
CONAN_EXTENSIONS_DIR = path.join(CONAN_ROOT, "conan_extensions")
CONAN_BUILD_INFO = {}

local function parseConanReference(ref)
	return ref:match("^([^@/]*)/?([^@/]*)@?([^@/]*)/?([^@/]*)$")
end

do -- Build the CONAN_PROFILES table
	local profileTemplate = {
		_HAS_BEEN_BUILT = true,
		build = true,
		includes = {},
		settings = {},
		options = {},
		-- We could also have a vars section for full feature parity, but
		-- since we can just do vars in lua so i dont think they are needed.
	}

	local function buildConanProfile(name)
		local prof = CONAN_PROFILES[name]
		assert(prof, "Unknown profile ".. tostring(name))
		if prof._HAS_BEEN_BUILT then return prof end

		local incs = {profileTemplate}

		for _, n in pairs(prof.includes or {}) do
			table.insert(incs, buildConanProfile(n))
		end

		table.insert(incs, prof)
		CONAN_PROFILES[name] = table.merge(table.unpack(incs))
		return CONAN_PROFILES[name]
	end

	for name in pairs(CONAN_PROFILES) do
		buildConanProfile(name)
	end
end

local function getCommandPrefix()
	local envs = {
		["CONAN_HOME"] = CONAN_HOME,
	}

	local host = os.host()
	local parts = {}

	if host == "windows" then
		for k,v in pairs(envs) do
			table.insert(parts, ("set \"%s=%s\"&& "):format(k,v))
		end
		table.insert(parts, "call ")
	elseif host == "linux" then
		error("Untested on linux")
		--for k,v in pairs(envs) do
		--	table.insert(parts, ("export '%s=%s'&& "):format(k,v))
		--end
	else
		error("Unsupported host \"".. tostring(host) .."\"")
	end

	return table.concat(parts)
end

local function execConan(cmd, output)
	local exec = ([[%s"%s" %s 2>&1]]):format(getCommandPrefix(), CONAN_EXE, cmd)
	print("execConan:", exec)
	return (output and os.outputof or os.execute)(exec)

	-- Could wrap with io.popen but ANSI color codes dont work unless run through echo.
	-- If you do end up doing this it would probably be worth pulling out into its own function so you can use it elsewhere
	--local out = io.popen(cmd, "r")
	--for line in out:lines() do
	--	os.execute("echo [CONAN] ".. res) -- Would need to escape special chars with ^ see: https://ss64.com/nt/syntax-esc.html
	--end
	--out:close()
	--for line in out:lines() do
	--	io.write("[CONAN] ", line, "\n")
	--end

end

function commands.remote()
	execConan("remote remove *")

	for k,v in pairs(CONAN_REMOTES) do
		execConan(("remote add %s %s"):format(k, v))
	end
end

function commands.export()
	for _, ref in pairs(CONAN_PACKAGES.requires) do
		local name, version, user, channel = parseConanReference(ref)
		io.write("\nExporting conan recipe ", ref, "\n")

		local dir = path.join(CONAN_RECIPES_DIR, name)
		assert(os.isdir(dir), "No recipe found with name ".. tostring(name))
		execConan(([[export %s --name "%s" --version "%s" --user "%s" --channel "%s"]]):format(dir, name, version, user, channel))
	end
end

function commands.install()
	if os.isdir(CONAN_BUILD_DIR) then
		assert(os.rmdir(CONAN_BUILD_DIR))
	end

	-- TODO: is this still true with Conan 2.0? Look into if we can avoid this
	-- We have to do it this way (w/ temp file) or else we can't use custom generators.
	local conanFilePath = CONAN_HOME .."/.temp.conanfile.".. os.uuid()
	do
		local file = io.open(conanFilePath, "w")

		for section, lines in pairs(CONAN_PACKAGES) do
			file:write("[", section, "]\n")
			for _, line in pairs(lines) do
				file:write(line, "\n")
			end
			file:write("\n")
		end
		file:close()
	end

	local errors = 0
	-- TODO: put all this in pcall so that we always clean up the temp file even if error
	for name, prof in pairs(CONAN_PROFILES) do
		if prof.build then
			local function buildArgs(arg, key, dat)
				local tbl = {}
				for k,v in pairs(dat) do
					table.insert(tbl, arg)

					if key then
						table.insert(tbl, k)
						table.insert(tbl, "=")
					end

					-- TODO: temp work around for https://github.com/conan-io/conan/issues/3620
					if type(v) == "boolean" then
						v = v and "True" or "False"
					end
					table.insert(tbl, v)
				end
				return table.concat(tbl)
			end

			local settings = buildArgs(" -s:a ", true, prof.settings)
			local options = buildArgs(" -o:a ", true, prof.options)
			local options = buildArgs(" -c:a ", true, prof.conf)
			local gens = buildArgs(" -g ", false, CONAN_PACKAGES.generators)
			local out = path.join(CONAN_BUILD_DIR, name)
			local jsonfile = path.join(out, "info.json")
			local err = execConan(
				("install -b missing -of %s/%s%s%s%s %s")
				:format(CONAN_BUILD_DIR, name, settings, options, gens, conanFilePath)
			)
			errors = errors + (err and 0 or 1)
		end
	end

	-- Remove Conan env bat file bloat. Originally these files weren't ever
	-- generated. Then they were generated but you could disable them. Now it
	-- looks like they have removed that ability: https://github.com/conan-io/conan/issues/13751
	for _, ext in pairs({"bat", "sh"}) do
		for _, file in pairs(os.matchfiles(path.join(CONAN_BUILD_DIR, "**.".. ext))) do
			os.remove(file)
		end
	end

	assert(os.remove(conanFilePath))
	return errors
end

function commands.exec()
	assert(#_ARGS >= 2, "exec requires additional arguments to pass to conan")
	execConan(table.concat(_ARGS, " ", 2, #_ARGS))
end

function commands.config()
	local extensionsPath = path.join(CONAN_HOME, "extensions")
	local defaultProfilePath = path.join(CONAN_HOME, "profiles", "default")

	-- TODO: look into if we can just leave the file blank, if not just use
	--       `conan profile detect` and check the we specify all of the settings in
	--       the profile settings in the loop below. At this point it might just be
	--       better to generate profile files instead and not have a default at all.
	os.remove(defaultProfilePath)
	io.writefile(defaultProfilePath,
[[
[settings]
arch=x86_64
build_type=Release
compiler=msvc
compiler.cppstd=14
compiler.runtime=dynamic
compiler.version=193
os=Windows
]]
	)

	-- Remove existing/stale extensions
	os.rmdir(extensionsPath)

	-- Force conan to re-generate the default extensions on the next conan command
	os.remove(path.join(CONAN_HOME, "version.txt"))

	-- Copy extensions over
	local files = os.matchfiles(path.join(CONAN_EXTENSIONS_DIR, "**"))
	for k,file in pairs(files) do
		local ext = path.getrelative(CONAN_EXTENSIONS_DIR, file)
		local dst = path.join(extensionsPath, ext)
		io.write("Installing extension: ", ext, "\n")
		os.mkdir(path.getdirectory(dst))
		assert(os.copyfile(file, dst))
	end
end

function commands.fullsetup()
	commands.config()
	commands.remote()
	commands.export()
	commands.install()
end

newaction {
	trigger = "conan",
	description = "TODO: desc",
	execute = function()
		local cmdstr = _ARGS[1]
		local cmd = commands[cmdstr]

		-- TODO: forward args to commands? for example `premake5 conan export -k` would be useful for testing
		if cmd then
			cmd()
		else
			error("Unknown conan command: ".. cmdstr)
		end

		--for k,v in pairs(_ARGS) do
		--	print(k, v)
		--end

		--for k,v in pairs(_OPTIONS) do
		--	print(k, v)
		--end
	end
}

local skiplist = {"conan", "clean"}
if not table.contains(skiplist, _ACTION) then
	if not os.isdir(CONAN_BUILD_DIR) then
		msg.warn("Unable to find conan build info. Attempting to install.\n")
		if commands.install() > 0 then
			msg.warn("One or more packages missing. Attempting to perform full setup.")
			commands.fullsetup()
		end
	end

	-- Workaround for premake ignoring loadfile's env parameter: https://github.com/premake/premake-core/issues/1392
	local oldmeta = getmetatable(_ENV)
	setmetatable(_ENV, {
		__newindex = function(t, k, v)
			print(_ENV, t, k, v)
			error("Conan build info assigned a non-local value. This is a generator bug.")
		end,
	});

	for name, prof in pairs(CONAN_PROFILES) do
		if prof.build then
			local file = CONAN_BUILD_DIR .."/".. name .."/conanbuildinfo.lua"
			if not os.isfile(file) then
				msg.warn("Unable to find conan build info for profile ", name, " \n")
			else
				CONAN_BUILD_INFO[name] = assert(loadfile(file))()
			end
		end
	end

	setmetatable(_ENV, oldmeta)
end
