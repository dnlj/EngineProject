from conans import ConanFile, tools, errors
import shutil

class Recipe(ConanFile):
	name = "dear_imgui"
	description = "Bloat-free Immediate Mode Graphical User interface for C++ with minimal dependencies."
	license = "MIT"
	homepage = "https://github.com/ocornut/imgui"
	url = "none"
	
	def source(self):
		isver = self.version[0].isdigit()
		tools.Git().clone(
			url="https://github.com/ocornut/imgui.git",
			branch=("v" if isver else "") + self.version,
			shallow=True
		)
		shutil.rmtree(".github")
		shutil.rmtree("docs")
		shutil.rmtree("examples")
		shutil.rmtree("misc")
		
	def build(self):
		pass
	
	def package(self):
		self.copy("*.h", src="", dst="include", keep_path=True)
		self.copy("*.cpp", src="", dst="src", keep_path=True)
		
	def package_id(self):
		self.info.header_only()
		
	def package_info(self):
		self.cpp_info.srcdirs = ["src"]
