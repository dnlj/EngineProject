from conans import ConanFile, tools, errors, CMake
import os

class Recipe(ConanFile):
	name = "freetype"
	description = "A freely available software library to render fonts."
	license = "FTL, GPLv2"
	homepage = "https://www.freetype.org/"
	url = "none"
	settings = "arch", "build_type", "compiler"
	
	def source(self):
		tools.Git().clone(
			url="https://gitlab.freedesktop.org/freetype/freetype.git",
			branch=self.version,
			shallow=True
		)
		
	def build(self):
		cmake = CMake(self)
		# FreeType enforces out of source builds
		cmake.configure(build_dir="build")
		cmake.build(build_dir="build")
		
	def package(self):
		self.copy("*.h", src="include", dst="include", keep_path=True)
		self.copy("*.lib", src="", dst="lib", keep_path=False)
		self.copy("*.a", src="", dst="lib", keep_path=False)
		
	def package_info(self):
		self.cpp_info.libs = tools.collect_libs(self)
