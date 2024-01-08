from conans import ConanFile, tools, errors, MSBuild
import os

class Recipe(ConanFile):
	name = "soil"
	description = "A tiny C library used primarily for uploading textures into OpenGL."
	license = "Public Domain"
	homepage = "http://www.lonesock.net/soil.html"
	url = "none"
	settings = "arch", "build_type", "compiler"
	
	def source(self):
		tools.get("http://www.lonesock.net/files/soil.zip")
		os.rename("Simple OpenGL Image Library", "soil")
		
	def build(self):
		if self.settings.compiler == "Visual Studio":
			msbuild = MSBuild(self)
			msbuild.build("soil/projects/VC9/SOIL.sln")
		else:
			raise Exception(f"Unimplemented compiler {self.settings.compiler}")
		
	def package(self):
		self.copy("*.h", src="soil/src", dst="include/soil", keep_path=True)
		self.copy("*.lib", src="soil/projects", dst="lib", keep_path=False)
		self.copy("*.a", src="soil/projects", dst="lib", keep_path=False)
		
	def package_info(self):
		self.cpp_info.libs = tools.collect_libs(self)
