function printTable(tbl, depth, shown)
	shown = shown or {}
	depth = depth or 0
	if depth == 0 then io.write(tostring(tbl), " = ") end
	depth = depth + 1
	
	local indent0 = ("  "):rep(depth - 1) 
	local indent = ("  "):rep(depth)
	io.write("{\n")
	for k,v in pairs(tbl) do
		if type(k) == "string" then
			io.write(indent, "\"", tostring(k), "\" = ")
		else
			io.write(indent, tostring(k), " = ")
		end
		
		local t = type(v)
		if t == "table" then
			if not shown[v] then
				shown[v] = true
				printTable(v, depth, shown)
			else
				io.write(tostring(tbl), " (repeat)\n")
			end
		elseif t == "string" then
			io.write("\"", tostring(v), "\"\n")
		else
			io.write(tostring(v), "\n")
		end
	end
	io.write(indent0, "}\n")
end