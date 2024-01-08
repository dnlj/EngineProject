from pprint import pprint
from conan import tools
from conans import model
import collections

# https://stackoverflow.com/a/1118038
# for debugging
def todict(obj, depth=1):
	"""
	Recursively convert a Python object graph to sequences (lists)
	and mappings (dicts) of primitives (bool, int, float, string, ...)
	"""
	if isinstance(obj, str) or depth > 7:
		return obj
	elif isinstance(obj, dict):
		return dict((key, todict(val, depth+1)) for key, val in obj.items())
	elif isinstance(obj, collections.Iterable):
		return [todict(val, depth+1) for val in obj]
	elif hasattr(obj, '__dict__'):
		return todict(vars(obj), depth+1)
	elif hasattr(obj, '__slots__'):
		return todict(dict((name, getattr(obj, name)) for name in getattr(obj, '__slots__')), depth+1)
	return obj

def makeSafeKey(s):
	return f'["{s}"]'

def toLua(obj, level = 1, prefix = None, *, seq=None):
	if seq is None:
		seq = level > 0

	indent = "\t" * level
	# Primitive objects
	if obj is None:
		return ["nil"]
	if isinstance(obj, bool):
		return ["true" if obj else "false"]
	if isinstance(obj, (int, float)):
		return [str(obj)]
	if isinstance(obj, str):
		return ["\"" + obj.replace("\\", "\\\\").replace("\"", "\\\"") + "\""]
	if isinstance(obj, collections.abc.KeysView):
		return toLua(list(obj), level)

	# Table like objects
	parts = []
	def addValue(v, k = None):
		parts.append(indent)
		if k:
			if prefix: parts.append(prefix)
			parts.append(makeSafeKey(k))
			parts.append(" = ")
		parts.extend(toLua(v, level + 1))
		parts.append(",\n") if seq else parts.append("\n")

	if seq: parts.append("{\n")

	if isinstance(obj, (list, tuple)):
		for v in obj:
			addValue(v)
	elif isinstance(obj, model.settings.Settings):
		for (k, v) in obj.values_list: addValue(v, k)
	elif isinstance(obj, collections.abc.ItemsView):
		for (k, v) in obj: addValue(v, k)
	elif isinstance(obj, dict):
		for (k, v) in sorted(obj.items()): addValue(v, k)
	elif isinstance(obj, model.build_info.CppInfo):
		# Convert to dict and try again
		# Conan always adds an extra "root" node just for fun.
		return toLua(obj.serialize()["root"], level)
	else:
		for k in dir(obj):
			v = getattr(obj, k)
			if k.startswith("_") or callable(v): continue
			addValue(v, k)
	if seq:
		parts.append("\t" * (level - 1))
		parts.append("}")
	return parts if (seq and len(parts) > 3) or (not seq and len(parts) > 0) else ["{}"]

def divider(title):
	return "-" * 80 + "\n-- " + title + "\n" + "-" * 80 + "\n"

def accumulateDeps(deps):
	"""
		Collect the cpp_info for all dependencies along with an collection of
		the accumulated info for all dependencies like conan use to provide.
	"""
	deps = list(deps.items())
	all = model.build_info.CppInfo()
	res = { "dependencies": {} }
	for req, dep in deps:
		# Not 100% sure what `aggregated_components` does here. I assume my
		# recipes are simple enough that it doesn't matter? Seems to work
		# fine without it.
		all.merge(dep.cpp_info)
		dep = dep.cpp_info.aggregated_components().serialize()
		res["dependencies"][req.ref] = dep["root"]
	res["_all"] = all.aggregated_components().serialize()["root"]
	return res

class premake5:
	def __init__(self, conanfile):
		self.conanfile = conanfile

	def generate(self):
		parts = []

		parts.append("local conan = {}\n\n")

		parts.append(divider("Helpers"))
		parts.append("\n".join([
			'function conan.setup()',
			'	for _, p in pairs(conan.src_paths) do',
			'		files(path.join(p, "**"))',
			'	end',
			'	',
			'	includedirs(conan.includedirs)',
			'	libdirs(conan.libdirs)',
			'	links(conan.libs)',
			'	bindirs(conan.bindirs)',
			'	defines(conan.defines)',
			'end\n\n',
		]))

		parts.append(divider("Settings"))
		parts.extend(toLua(self.conanfile.settings, 0, "conan"))
		parts.append("\n")

		# I think we only care about "host" deps atm. There are also `.build` and `.test`.
		parts.append(divider("Host Info"))
		host = accumulateDeps(self.conanfile.dependencies.host)
		parts.extend(toLua(host["_all"], 0, "conan"))
		parts.extend(toLua({"dependencies": host["dependencies"]}, 0, "conan"))

		parts.append(divider("Build Info"))
		build = accumulateDeps(self.conanfile.dependencies.build)
		parts.extend(toLua(build["_all"], 0, "conan"))
		parts.extend(toLua({"dependencies": build["dependencies"]}, 0, "conan"))

		parts.append(divider("Test Info"))
		test = accumulateDeps(self.conanfile.dependencies.test)
		parts.extend(toLua(test["_all"], 0, "conan"))
		parts.extend(toLua({"dependencies": test["dependencies"]}, 0, "conan"))

		parts.append("return conan")

		tools.files.save(self.conanfile, "conanbuildinfo.lua", "".join(parts))
