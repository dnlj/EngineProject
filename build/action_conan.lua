assert(CONAN_USER_HOME, "CONAN_USER_HOME must be defined")
assert(CONAN_PACKAGES, "CONAN_PACKAGES must be defined")
assert(CONAN_PROFILES, "CONAN_PROFILES must be defined")

local subCommands = {}
local templateDir = CONAN_USER_HOME .."/conan_template"
local recipesDir = CONAN_USER_HOME .."/conan_recipes"

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
		env = {},
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

local getCommandPrefix
do
	local cmd
	function getCommandPrefix()
		if cmd then return cmd end
		
		local envs = {
			["CONAN_USER_HOME"] = CONAN_USER_HOME,
			
			-- CONAN_COLOR_DISPLAY doesn't actually force color display. PYCHARM_HOSTED is a workaround
			-- https://github.com/conan-io/conan/pull/4885
			-- https://github.com/conan-io/conan/issues/4718
			--["CONAN_COLOR_DISPLAY"] = 1,
			--["PYCHARM_HOSTED"] = 1, 
		}
		
		local host = os.host()
		local parts = {}
		local prefix
		
		if host == "windows" then
			for k,v in pairs(envs) do
				table.insert(parts, ("set \"%s=%s\"&& "):format(k,v))
			end
			table.insert(parts, "call ")
		elseif host == "linux" then
			for k,v in pairs(envs) do
				table.insert(parts, ("export '%s=%s&&' "):format(k,v))
			end
		else
			error("Unsupported host \"".. tostring(host) .."\"")
		end
		
		prefix = table.concat(parts)
		return prefix
	end
end

local function execConan(cmd)
	os.execute(getCommandPrefix() .. cmd .." 2>&1")
	
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

function subCommands.remote()
	execConan("conan remote clean")
	for k,v in pairs(CONAN_REMOTES) do
		execConan(("conan remote add %s %s"):format(k, v))
	end
end

function subCommands.export()
	local dirs = os.matchdirs(recipesDir .."/*")
	for _, ref in pairs(CONAN_PACKAGES.requires) do
		local name, version, user, channel = parseConanReference(ref)
		io.write("\nExporting conan recipe ", ref, "\n")
		
		local dir = recipesDir .."/".. name
		assert(os.isdir(dir), "No recipe found with name ".. tostring(name))
		
		execConan(("conan export %s %s"):format(dir, ref))
	end
end

function subCommands.install()
	local fileName = CONAN_USER_HOME .."/temp_conanfile.txt_".. os.uuid()
	local file = io.open(fileName, "w")
	
	for section, lines in pairs(CONAN_PACKAGES) do
		file:write("[", section, "]\n")
		for _, line in pairs(lines) do
			file:write(line, "\n")
		end
		file:write("\n")
	end
	file:close()
	
	for name, prof in pairs(CONAN_PROFILES) do
		if prof.build then
			local function buildArgs(arg, dat)
				local tbl = {}
				for k,v in pairs(dat) do
					table.insert(tbl, arg)
					table.insert(tbl, k)
					table.insert(tbl, "=")
					table.insert(tbl, v)
				end
				return table.concat(tbl)
			end
			
			local settings = buildArgs(" -s ", prof.settings)
			local options = buildArgs(" -o ", prof.options)
			local envs = buildArgs(" -e ", prof.env)
			
			for _, ref in pairs(CONAN_PACKAGES.requires) do
				execConan(("conan install -b -if conan_build_%s%s%s%s %s"):format(name, settings, options, envs, fileName))
			end
		end
	end
	
	assert(os.remove(fileName))
end

newaction {
	trigger = "conan",
	description = "TODO: desc",
	execute = function()
		local cmdstr = _ARGS[1]
		local cmd = subCommands[cmdstr]

		if cmd then
			cmd()
		else
			-- TODO: Error message
			-- search commands for contains cmdstr
			print("No cmd?")
		end

		--for k,v in pairs(_ARGS) do
		--	print(k, v)
		--end

		--for k,v in pairs(_OPTIONS) do
		--	print(k, v)
		--end
	end
}

if _ACTION ~= "conan" then
	-- TODO: Only try to load files if they exist. If they dont maybe throw a warning? For example we dont need these files if we do a `premake5 clean`
	-- Workaround for premake ignoring loadfile's env parameter: https://github.com/premake/premake-core/issues/1392
	local oldmeta = getmetatable(_ENV)
	local values = {}
	setmetatable(_ENV, {
		__newindex = function(t, k, v)
			values[k] = v
		end
	});

	assert(loadfile("conanbuildinfo.premake.lua"))()
	setmetatable(_ENV, oldmeta)

	for k,v in pairs(values) do print("--- ", k, v) end

	print("premake _ENV: ", _ENV)
	print("premake env: ", env)
end
