from conans import ConanFile, tools, errors
import conans.model as model
import collections

def makeSafeKey(s):
	# We could also wrap keys like ["key.text.here"] but that isnt very nice as a consumer
	return s.replace(".", "_")
	
def toLua(obj, level = 1, prefix = None): # TODO: return a list of parts?
	seq = level > 0
	indent = "\t" * level
	
	# Primitive objects
	if obj is None:
		return ["nil"]
	if isinstance(obj, (int, float)):
		return [str(obj)]
	if isinstance(obj, bool):
		return ["true" if obj else "false"]
	if isinstance(obj, str):
		# TODO: also need to check if contains `"` char. There may already be a function for this.
		return ["\"" + obj.replace("\\", "\\\\") + "\""]
	if isinstance(obj, collections.abc.KeysView):
		return toLua(list(obj), level + 1)
	
	# TODO: simplify duplication
	# Table like objects
	parts = []
	if seq: parts.append("{\n")
	if isinstance(obj, (list, tuple)):
		for v in obj:
			parts.append(indent)
			parts.extend(toLua(v, level + 1))
			parts.append(",\n") if seq else parts.append("\n")
	elif isinstance(obj, model.settings.Settings):
		for (k, v) in obj.values_list:
			parts.append(indent)
			if prefix: parts.append(prefix)
			parts.append(makeSafeKey(k))
			parts.append(" = ")
			parts.extend(toLua(v, level + 1))
			parts.append(",\n") if seq else parts.append("\n")
	elif isinstance(obj, collections.abc.ItemsView):
		for (k, v) in obj:
			parts.append(indent)
			if prefix: parts.append(prefix)
			parts.append(makeSafeKey(k))
			parts.append(" = ")
			parts.extend(toLua(v, level + 1))
			parts.append(",\n") if seq else parts.append("\n")
	else:
		for k in dir(obj):
			v = getattr(obj, k)	
			if k.startswith("_") or callable(v): continue
			parts.append(indent)
			if prefix: parts.append(prefix)
			parts.append(makeSafeKey(k))
			parts.append(" = ")
			parts.extend(toLua(v, level + 1))
			parts.append(",\n") if seq else parts.append("\n")
	if seq:
		parts.append("\t" * (level - 1))
		parts.append("}")
	return parts if (seq and len(parts) > 3) or (not seq and len(parts) > 0) else ["{}"]

def divider(title):
	return "-" * 80 + "\n-- " + title + "\n" + "-" * 80 + "\n"
	
class premake5(model.Generator):
	@property
	def filename(self):
		print("\n\n** filename")
		return "wooples.lua"
		
	@property
	def content(self):
		parts = []
		parts.append("conan = {}\n\n")
		print("\n\n************************ Begin content")
		parts.append(divider("Settings"))
		parts.extend(toLua(self.settings, 0, "conan."))
		parts.append("\n")
		parts.append(divider("Build Info"))
		parts.extend(toLua(self.deps_build_info, 0, "conan."))
		return "".join(parts)
		
class Package(ConanFile):
	name = "premake5"
	description = "Generator for premake5"
	license = "none"
	homepage = "none"
	url = "none"
	
	def build(self):
		pass
	
	def package(self):
		# TODO: is there any way to disable `WARN: No files in this package!`
		pass
	
	def package_id(self):
		self.info.header_only()
		
	def package_info(self):
		pass
