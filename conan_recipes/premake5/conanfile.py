from conans import ConanFile, tools, errors
from conans.model import Generator

# TODO: indent

def toLua(obj):
	if isinstance(obj, list):
		lines = []
		lines.append("{\n")
		for o in obj:
			lines.append(toLua(o))
			lines.append(",\n")
		lines.append("}")
		return "".join(lines)
	# TODO: odict_items
	# TODO: odict_keys
	if obj is None:
		return "nil"
	# TODO: bools
	# TODO: numbers
	return "\"" + str(obj).replace("\\", "\\\\") + "\""

def divider(title):
	return "-" * 80 + "\n-- " + title + "\n" + "-" * 80 + "\n"
	
class premake5(Generator):
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
		for (k, v) in self.settings.values_list:
			print(k, v)
			parts.append("conan.")
			parts.append(k.replace(".", "_"))
			parts.append(" = ")
			parts.append(toLua(v))
			parts.append("\n")
		print("==========================================")
		parts.append("\n")
		parts.append(divider("Build Info"))
		info = self.deps_build_info
		for k in dir(info):
			if k.startswith("_") or callable(getattr(info, k)): continue
			print(k, "-", getattr(info, k))
			parts.append("conan.")
			parts.append(k)
			parts.append(" = ")
			parts.append(toLua(getattr(info, k)))
			parts.append("\n")
		# TODO: settings:
		#print("------------------------------------------")
		#for k in dir(self):
		#	if k.startswith("_") or callable(getattr(info, k)): continue
		#	print(k)
		print("************************ End content")
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
		pass
	
	def package_id(self):
		self.info.header_only()
		
	def package_info(self):
		pass
