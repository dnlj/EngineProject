from conans import ConanFile, tools, errors, CMake
import os

cmakedata = """
cmake_minimum_required(VERSION 3.16)
project(soil_littlstar)
file(GLOB_RECURSE SOURCES *.cpp *.c *.h *.hpp)
add_library(soil_littlstar STATIC ${SOURCES})
include_directories(soil_littlstar include)
"""

class Recipe(ConanFile):
	name = "soil_littlstar"
	description = "A tiny C library used primarily for uploading textures into OpenGL."
	license = "Public Domain"
	homepage = "https://github.com/littlstar/soil"
	url = "none"
	settings = "arch", "build_type", "compiler"
	
	def source(self):
		tools.Git().clone(
			url="https://github.com/littlstar/soil.git",
			branch=self.version,
			shallow=True
		)
		
		with open("CMakeLists.txt", "w") as f:
			f.write(cmakedata)
		
	def build(self):
		cmake = CMake(self)
		cmake.configure()
		cmake.build()
		
	def package(self):
		self.copy("*.h", src="", dst="include/soil", keep_path=False)
		self.copy("*.lib", src="", dst="lib", keep_path=False)
		self.copy("*.a", src="", dst="lib", keep_path=False)
		
	def package_info(self):
		self.cpp_info.libs = tools.collect_libs(self)
