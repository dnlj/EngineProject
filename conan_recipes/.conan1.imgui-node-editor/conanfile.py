from conans import ConanFile, tools, errors, CMake
import shutil

cmakedata = """
cmake_minimum_required(VERSION 3.16)
project(imgui-node-editor)
include(conanbuildinfo.cmake)
conan_basic_setup()
file(GLOB SOURCES *.cpp *.h *.inl)
add_library(imgui-node-editor STATIC ${SOURCES})
"""

class Recipe(ConanFile):
	name = "imgui-node-editor"
	description = "An implementation of node editor with ImGui-like API."
	license = "MIT"
	homepage = "https://github.com/thedmd/imgui-node-editor"
	generators = "cmake"
	requires = "imgui/master@dnlj/wobbly"
	url = "none"
	settings = "arch", "build_type", "compiler"
	
	def source(self):
		isver = self.version[0].isdigit()
		tools.Git().clone(
			url="https://github.com/thedmd/imgui-node-editor.git",
			branch=("v" if isver else "") + self.version,
			shallow=True
		)
		
		shutil.rmtree("docs")
		shutil.rmtree("examples")
		shutil.rmtree("external")
		shutil.rmtree("misc")
		
		with open("CMakeLists.txt", "w") as f:
			f.write(cmakedata)
		
	def build(self):
		cmake = CMake(self)
		cmake.configure()
		cmake.build()
	
	def package(self):
		self.copy("*.h", src="", dst="include", keep_path=True)
		self.copy("*.inl", src="", dst="include", keep_path=True)
		self.copy("*.lib", src="", dst="lib", keep_path=False)
		self.copy("*.a", src="", dst="lib", keep_path=False)
		
	def package_info(self):
		self.cpp_info.libs = tools.collect_libs(self)
		
