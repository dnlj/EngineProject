-- Project Information
local depsDir = "./deps/"
local cacheDir = depsDir .."_dlcache/"

-- Load packages
local PACKAGES = loadfile("./packages.lua")(depsDir)


--------------------------------------------------------------------------------
-- Download and Build actions
--------------------------------------------------------------------------------
local function correctPermissions(dir)
	local files = os.matchfiles(dir .."**")
	
	for k,v in pairs(files) do
		os.chmod(v, "777")
	end
end

local function progress(total, current)
	local barLength = 61
	local ratio = current / total
	local percent = math.floor(ratio * 100)
	local completeLength = math.floor(barLength * ratio)
	local complete = string.rep("#", completeLength)
	local incomplete = string.rep(".", barLength - completeLength)
	io.write("\rProgress: ", string.format("[%3d%%]", percent), " [", complete, incomplete, "]")
end

local function download(uid, data, nocache)
	local workingDir = os.getcwd()
	local extractDir = depsDir .. uid .."/"
	local file = cacheDir .. uid .. ".zip"
	
	io.write("\nDownloading: ", data.name, "\n")
	
	-- Make sure the cache directroy exists
	os.mkdir(cacheDir)
	
	-- Delete any old versions
	os.rmdir(extractDir)
	
	-- Redownload if needed
	if nocache then
		os.remove(file)
	end
	
	-- Download
	if not os.isfile(file) then
		local result, response = http.download(data.url, file, {progress = progress})			
		
		if result ~= "OK" then
			os.remove(file)
			io.write("ERROR: ", result)
		else
			os.chmod(file, "777")
			io.write("\nComplete.\n")
		end
	else
		io.write("Cached.\n")
	end
	
	-- Extract
	zip.extract(file, extractDir ..".") -- The dot is need because zip.extract doesn't like paths ending with a slash
	correctPermissions(extractDir)
	
	if data.postExtract ~= nil then
		data.postExtract(uid)
	end
	
	os.chdir(workingDir)
end

local function build(uid, data)
	local workingDir = os.getcwd()
	
	io.write("Building: ", uid, "\n")
	data.build(uid)
	
	os.chdir(workingDir)
end

newaction {
	trigger = "download-deps",
	description = "Download dependencies.",
	execute = function()
		io.write("Downloading dependencies...\n")
		local nocache = false
		local skip = false
		
		for k,v in pairs(_ARGS) do
			if v == "-nocache" then
				nocache = true
				break
			end
		end
		
		-- We must use ipairs here because each arg has a index entry and a key entry
		for k,v in ipairs(_ARGS) do
			if string.sub(v, 1, 1) ~= "-" then
				skip = true
				local arg = string.lower(v)
				local pack = PACKAGES[arg]

				if pack then
					download(arg, pack, nocache)
				else
					io.write("WARNING: Unknown dependency \"", arg, "\"\n")
				end
			end
		end
		
		if not skip then
			for k,v in pairs(PACKAGES) do
				download(k, v, nocache)
			end
		end
	end
}

newaction {
	trigger = "build-deps",
	description = "Build dependencies.",
	execute = function()
		io.write("Building dependencies...\n\n")
		local skip = false
		
		-- We must use ipairs here because each arg has a index entry and a key entry
		for k,v in ipairs(_ARGS) do
			if string.sub(v, 1, 1) ~= "-" then
				skip = true
				local arg = string.lower(v)
				local pack = PACKAGES[arg]

				if pack then
					build(arg, pack)
				else
					io.write("WARNING: Unknown dependency \"", arg, "\"\n")
				end
			end
		end
		
		if not skip then
			for k,v in pairs(PACKAGES) do
				build(k, v)
			end
		end
	end
}
