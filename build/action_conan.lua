assert(CONAN_USER_HOME, "CONAN_USER_HOME must be defined")
assert(CONAN_PACKAGES, "CONAN_PACKAGES must be defined")

local subCommands = {}
local templateDir = CONAN_USER_HOME .."/conan_template"
local recipesDir = CONAN_USER_HOME .."/conan_recipes"

local function parseConanReference(ref)
	return ref:match("^([^@/]*)/?([^@/]*)@?([^@/]*)/?([^@/]*)$")
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

local getProfiles
do
	local profiles
	function getProfiles()
		if profiles then return profiles end
		profiles = {}

		local files = os.matchfiles(templateDir .."/profiles/*")
		for _, file in pairs(files) do
			file = path.getname(file)
			if file:sub(1,1) ~= "." then
				table.insert(profiles, file)
			end
		end

		return profiles
	end
end

function subCommands.export()
	local dirs = os.matchdirs(recipesDir .."/*")
	for i, ref in pairs(CONAN_PACKAGES.requires) do
		local name, version, user, channel = parseConanReference(ref)
		io.write("\nExporting conan recipe ", ref, "\n")
		
		local dir = recipesDir .."/".. name
		assert(os.isdir(dir), "No recipe found with name ".. tostring(name))
		
		execConan(("conan export %s %s"):format(dir, ref))
	end
end

function subCommands.config()
	execConan("conan config install ".. templateDir)
end

function subCommands.install()
	os.execute("echo install")
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
